/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "ITKImage.h"

bool CITKImage::LoadFromFile(const char* filename)
{
    m_filename = filename;

    QFileInfo info(m_filename);
    m_isDicom = info.suffix().toLower() == "dcm" || info.suffix().toLower() == "dicom";

    itk::ImageIOBase::Pointer imageIO =
        itk::ImageIOFactory::CreateImageIO(m_filename, itk::CommonEnums::IOFileMode::ReadMode);

    imageIO->SetFileName(m_filename);
    imageIO->ReadImageInformation();

    pixelType = imageIO->GetPixelType();
    componentType = imageIO->GetComponentType();
    const unsigned int imageDimension = imageIO->GetNumberOfDimensions();

    switch (pixelType)
    {
        case itk::IOPixelEnum::SCALAR:
        {
            if (imageDimension == 3)
            {
                if(!ReadScalarImage()) return false;
                break;
            }
            else
            {
                return false;
            }
            
        }
        default:
        {
            std::cerr << "not implemented yet!" << std::endl;
            return false;
        }
            
    }

    m_pb = finalImage->GetBufferPointer();

    const typename FinalImageType::SizeType& sz = finalImage->GetBufferedRegion().GetSize();
    m_cx = sz[0];
    m_cy = sz[1];
    m_cz = sz[2];

    typename FinalImageType::SpacingType spacing = finalImage->GetSpacing();

    std::cout << spacing[0] << " " << spacing[1] << " " << spacing[2] << std::endl;

    typename FinalImageType::PointType origin = finalImage->GetOrigin();
    std::cout << origin[0] << " " << origin[1] << " " << origin[2] << std::endl;

}

template <class TImage>
bool
CITKImage::ReadImage()
{
    using ImageType = TImage;
    
    using SeriesReaderType = itk::ImageSeriesReader<ImageType>;
    typename SeriesReaderType::Pointer seriesReader;
    using ImageIOType = itk::GDCMImageIO;
    ImageIOType::Pointer gdcmImageIO;

    using ImageReaderType = itk::ImageFileReader<ImageType>;
    typename ImageReaderType::Pointer reader;

    using RescaleType = itk::RescaleIntensityImageFilter<ImageType, ImageType>;
    itk::SmartPointer<RescaleType> rescale = RescaleType::New();
    rescale->SetOutputMinimum(0);
    rescale->SetOutputMaximum(itk::NumericTraits<unsigned char>::max());

    if(m_isDicom)
    {
        using ImageIOType = itk::GDCMImageIO;
        gdcmImageIO = ImageIOType::New();
        
        seriesReader = SeriesReaderType::New();
        seriesReader->SetImageIO(gdcmImageIO);

        using NamesGeneratorType = itk::GDCMSeriesFileNames;
        itk::SmartPointer<NamesGeneratorType> nameGenerator = NamesGeneratorType::New();
        nameGenerator->SetUseSeriesDetails(true);

        QFileInfo info(m_filename);
        nameGenerator->SetDirectory(info.absolutePath().toStdString().c_str());

        using SeriesIdContainer = std::vector<std::string>;

        const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();

        auto seriesItr = seriesUID.begin();
        auto seriesEnd = seriesUID.end();
        while (seriesItr != seriesEnd)
        {
        std::cout << seriesItr->c_str() << std::endl;
            ++seriesItr;
        }
        std::string seriesIdentifier = seriesUID.begin()->c_str();

        std::cout << seriesIdentifier << std::endl;

        using FileNamesContainer = std::vector<std::string>;
        FileNamesContainer fileNames;

        fileNames = nameGenerator->GetFileNames(seriesIdentifier);
        seriesReader->SetFileNames(fileNames);

        rescale->SetInput(seriesReader->GetOutput());
    }
    else
    {
        reader = ImageReaderType::New();   
        reader->SetFileName(m_filename);

        rescale->SetInput(reader->GetOutput());
    }

    using CastType = itk::CastImageFilter<ImageType, FinalImageType>;
    itk::SmartPointer<CastType> castFilter = CastType::New();
    castFilter->SetInput(rescale->GetOutput());

    try
    {
        castFilter->Update();
    }
    catch (const itk::ExceptionObject & e)
    {
        std::cerr << "exception in file reader " << std::endl;
        std::cerr << e << std::endl;
        return false;
    }

    finalImage = castFilter->GetOutput();

    return true;
}

int CITKImage::ReadScalarImage()
{
    switch (componentType)
    {
        default:
        case itk::IOComponentEnum::UNKNOWNCOMPONENTTYPE:
            std::cerr << "Unknown and unsupported component type!" << std::endl;
            return false;

        case itk::IOComponentEnum::UCHAR:
        {
            using PixelType = unsigned char;
            using ImageType = itk::Image<PixelType, 3>;

            // In this case, we don't need to convert the image type, so we just read it directly

            using ImageReaderType = itk::ImageFileReader<ImageType>;
            typename ImageReaderType::Pointer reader = ImageReaderType::New();
            reader->SetFileName(m_filename);
            
            try
            {
                reader->Update();
            }
            catch (const itk::ExceptionObject & e)
            {
                std::cerr << "exception in file reader " << std::endl;
                std::cerr << e << std::endl;
                return false;
            }

            finalImage = reader->GetOutput();

            return true;
        }

        case itk::IOComponentEnum::CHAR:
        {
            using PixelType = char;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::USHORT:
        {
            using PixelType = unsigned short;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::SHORT:
        {
            using PixelType = short;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::UINT:
        {
            using PixelType = unsigned int;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::INT:
        {
            using PixelType = int;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::ULONG:
        {
            using PixelType = unsigned long;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::LONG:
        {
            using PixelType = long;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::FLOAT:
        {
            using PixelType = float;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }

        case itk::IOComponentEnum::DOUBLE:
        {
            using PixelType = double;
            using ImageType = itk::Image<PixelType, 3>;

            return ReadImage<ImageType>();
        }
    }
    return EXIT_SUCCESS;

}