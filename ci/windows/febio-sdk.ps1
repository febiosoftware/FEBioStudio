$gh_token=$env:GH_TOKEN
$artifact_name=$env:ARTIFACT_NAME
$locate_archive_url="https://api.github.com/repos/febiosoftware/FEBio/actions/artifacts?name=$($artifact_name)&per_page=1"

function Get-Archive-Url {
  (iwr -Uri $locate_archive_url -Headers @{ 'Authorization' = 'Bearer ' + $gh_token} -UseBasicParsing ).content | jq -r .artifacts[0].archive_download_url
}

function Get-Archive($archive_url, $archive_name) {
  iwr -Uri $archive_url -Headers @{ 'Authorization' = 'Bearer ' + $gh_token} -OutFile $archive_name
}

function main {
  $archive_url = Get-Archive-Url
  $archive_name = "$($artifact_name).zip"
  Get-Archive $archive_url $archive_name
  unzip $archive_name -d febio-sdk
}

main
