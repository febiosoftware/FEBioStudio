#! /bin/bash
set -o errexit
set -o verbose

# sudo apt-get install jq -y
LOCATE_ARCHIVE_URL="https://api.github.com/repos/febiosoftware/FEBio/actions/artifacts?name=febio4-sdk-Linux-X64&per_page=1"

get_archive_url() {
	local locate_archive_url=$1
	echo $(curl -H "Accept: application/vnd.github+json" -H "Authorization: Bearer $GH_TOKEN" -H "X-GitHub-Api-Version: 2022-11-28" $locate_archive_url | jq -r .artifacts[0].archive_download_url)
}

get_archive() {
	local archive_url=$1
	local output_file=$2
	curl -L -H "Accept: application/vnd.github+json" -H "Authorization: Bearer $GH_TOKEN" -H "X-GitHub-Api-Version: 2022-11-28" $archive_url --output $output_file
}

main() {
	local archive_url=$(get_archive_url $LOCATE_ARCHIVE_URL)
	local output_file="febio4-sdk-Linux-X64.zip"
	get_archive "$archive_url" $output_file
	unzip "$output_file" -d "febio-sdk"
}

main
