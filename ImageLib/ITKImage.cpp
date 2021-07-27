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
#include <itkRescaleIntensityImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageSeriesReader.h>
// #include <itkImageIOBase.h>
#include <itkImageFileReader.h>
#include <itkGDCMImageIO.h>
#include <gdcmGlobal.h>
#include <itkMetaDataObject.h>

#include <QFileInfo>

bool CITKImage::LoadFromFile(const char* filename, ImageFileType type)
{
    m_filename = filename;
    m_type = type;

    switch(m_type)
    {
        case ImageFileType::SEQUENCE:
            GetNamesForSequence();
            m_imageFilename = sequenceFiles[0].c_str();
            break;
        default:
            m_imageFilename = filename;
    }

    if(!ParseImageHeader()) return false;

    if(!ReadScalarImage()) return false;   

    FinalizeImage();

    return true;
}

std::vector<int> CITKImage::GetSize()
{
    std::vector<int> size;
    FinalImageType::RegionType::SizeType itkSize = finalImage->GetBufferedRegion().GetSize();

    for(auto itksz : itkSize)
    {
        size.push_back(itksz);
    }

    return size;
}

std::vector<double> CITKImage::GetOrigin()
{
    std::vector<double> origin;
    FinalImageType::PointType itkOrigin = finalImage->GetOrigin();

    for(auto itkCoord : itkOrigin)
    {
        origin.push_back(itkCoord);
    }

    return origin;
}

std::vector<double> CITKImage::GetSpacing()
{
    std::vector<double> spacing;
    FinalImageType::SpacingType itkSpacing = finalImage->GetSpacing();

    for(auto itkCoord : itkSpacing)
    {
        spacing.push_back(itkCoord);
    }

    return spacing;
}

bool CITKImage::ParseImageHeader()
{
    itk::ImageIOBase::Pointer imageIO =
        itk::ImageIOFactory::CreateImageIO(m_imageFilename, itk::CommonEnums::IOFileMode::ReadMode);

    if(!imageIO)
    {
        std::cerr << "Cannot read image pixel type" << std::endl;
        return false;
    }

    imageIO->SetFileName(m_imageFilename);
    imageIO->ReadImageInformation();

    pixelType = imageIO->GetPixelType();
    if(pixelType != itk::IOPixelEnum::SCALAR)
    {
        std::cerr << "We only support scalar pixel types." << std::endl;
        return false;
    }
    
    
    componentType = imageIO->GetComponentType();
    
    if(imageIO->GetNumberOfDimensions() != 3)
    {
        std::cerr << "We only support 3D images." << std::endl;
            return false;
    }

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
            reader->SetFileName(m_imageFilename);
            
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

    using CastType = itk::CastImageFilter<ImageType, FinalImageType>;
    itk::SmartPointer<CastType> castFilter = CastType::New();

    if(m_type == ImageFileType::DICOM)
    {
        using ImageIOType = itk::GDCMImageIO;
        gdcmImageIO = ImageIOType::New();
        
        seriesReader = SeriesReaderType::New();
        seriesReader->SetImageIO(gdcmImageIO);

        using NamesGeneratorType = itk::GDCMSeriesFileNames;
        itk::SmartPointer<NamesGeneratorType> nameGenerator = NamesGeneratorType::New();
        nameGenerator->SetUseSeriesDetails(true);

        QFileInfo info(m_imageFilename);
        nameGenerator->SetDirectory(info.absolutePath().toStdString().c_str());

        using SeriesIdContainer = std::vector<std::string>;

        const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();

        using FileNamesContainer = std::vector<std::string>;
        FileNamesContainer fileNames;

        auto seriesItr = seriesUID.begin();
        auto seriesEnd = seriesUID.end();
        bool found = false;
        while (seriesItr != seriesEnd && !found)
        {
            fileNames = nameGenerator->GetFileNames(seriesItr->c_str());

            for(auto name : fileNames)
            {
                if(name == m_imageFilename)
                {
                    found = true;
                    break;
                }
            }

            ++seriesItr;
        }
        
        seriesReader->SetFileNames(fileNames);

        rescale->SetInput(seriesReader->GetOutput());

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

        const itk::MetaDataDictionary & dictionary = gdcmImageIO->GetMetaDataDictionary();
        using MetaDataStringType = itk::MetaDataObject<std::string>;

        FinalImageType::SpacingType spacing = finalImage->GetSpacing();

        std::string sliceSpacingID = "0018|0088";
        auto tagItr = dictionary.Find(sliceSpacingID);

        if(tagItr != dictionary.End())
        {
            MetaDataStringType::ConstPointer entryvalue =
                dynamic_cast<const MetaDataStringType *>(tagItr->second.GetPointer());

            if (entryvalue)
            {
                std::string tagvalue = entryvalue->GetMetaDataObjectValue();

                try
                {
                    spacing[2] = std::stod(tagvalue);
                }
                catch(const std::exception& e){}
            }
        }

        std::string pixelSpacingID = "0028|0030";
        tagItr = dictionary.Find(pixelSpacingID);

        if(tagItr != dictionary.End())
        {
            MetaDataStringType::ConstPointer entryvalue =
                dynamic_cast<const MetaDataStringType *>(tagItr->second.GetPointer());

            if (entryvalue)
            {
                std::string tagvalue = entryvalue->GetMetaDataObjectValue();

                int index = tagvalue.find("\\");

                
                try
                {
                    spacing[0] = std::stod(tagvalue.substr(0, index));
                }
                catch(const std::exception& e){}

                try
                {
                    spacing[1] = std::stod(tagvalue.substr(index+1));
                }
                catch(const std::exception& e){}
            }
        }

        finalImage->SetSpacing(spacing);

        FinalImageType::PointType origin = finalImage->GetOrigin();

        std::string originID = "0020|0032";
        tagItr = dictionary.Find(originID);

        if(tagItr != dictionary.End())
        {
            MetaDataStringType::ConstPointer entryvalue =
                dynamic_cast<const MetaDataStringType *>(tagItr->second.GetPointer());

            if (entryvalue)
            {
                std::string tagvalue = entryvalue->GetMetaDataObjectValue();

                int index1 = tagvalue.find("\\");
                int index2 = tagvalue.find("\\", index1+1);

                
                try
                {
                    origin[0] = std::stod(tagvalue.substr(0, index1));
                }
                catch(const std::exception& e){}

                try
                {
                    origin[1] = std::stod(tagvalue.substr(index1+1, index2-index1));
                }
                catch(const std::exception& e){}

                try
                {
                    origin[2] = std::stod(tagvalue.substr(index2));
                }
                catch(const std::exception& e){}
            }
        }

        finalImage->SetOrigin(origin);

    }
    else
    {
        reader = ImageReaderType::New();   
        reader->SetFileName(m_imageFilename);

        rescale->SetInput(reader->GetOutput());

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
    }

    return true;
}

void CITKImage::GetNamesForSequence()
{
    
}

bool CITKImage::FinalizeImage()
{
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