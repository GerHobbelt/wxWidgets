#!/bin/bash

calculate_md5()
{
    while IFS= read -r file_path; do
        file_name=$(basename "$file_path")
        hash=$(md5sum "$file_path" | awk '{print $1}')
        echo "$file_name: $hash"
    done
}

# Clean the file first if we've ran it before
> release_message.md

echo "Built from commit: $GITHUB_SHA" >> release_message.md

echo -e "\n### Hashes" >> release_message.md
find ./artifacts/ -type f | calculate_md5 >> release_message.md
