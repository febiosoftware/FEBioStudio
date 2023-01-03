$gh_token=$env:GH_TOKEN
$artifact_name=$env:ARTIFACT_NAME
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls -bor [Net.SecurityProtocolType]::Tls11 -bor [Net.SecurityProtocolType]::Tls12

$locate_archive_url="https://api.github.com/repos/febiosoftware/FEBio/actions/artifacts?name=$($artifact_name)&per_page=1"

$headers = @{ 'Authorization' = 'Bearer ' + $gh_token; 'X-Github-Api-Version' = '2022-11-28'; }

function Get-Archive-Url($headers, $locate_archive_url) {
  (iwr -Uri $locate_archive_url -Headers $headers  -UseBasicParsing ).content | jq -r .artifacts[0].archive_download_url
}

function Get-Archive($headers, $archive_url, $archive_name) {
  iwr -Uri $archive_url -Headers $headers -OutFile $archive_name
}

function main {
  $archive_url = Get-Archive-Url $headers $locate_archive_url
  $archive_name = "$($artifact_name).zip"
  Get-Archive $headers $archive_url $archive_name
  unzip $archive_name -d febio-sdk
}

main
