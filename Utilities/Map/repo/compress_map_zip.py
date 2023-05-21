#!/usr/bin/python

import zipfile
import os

# Set the directory where the input files are located
input_directory = "/media/horinek/topo_data/OSM/step4/"

# Set the directory where the output zip files should be saved
output_directory = "/media/horinek/topo_data/repo/map/"

# Get a list of all files in the input directory
input_files = os.listdir(input_directory)

# Iterate over each file in the input directory
for input_file in input_files:
    # Get the full path of the input file
    input_file_path = os.path.join(input_directory, input_file)

    # Create the filename for the output zip file
    output_file_name = os.path.splitext(input_file)[0] + ".ZIP"

    # Get the full path of the output zip file
    output_file_path = os.path.join(output_directory, output_file_name)

    # Create a new zip file in maximum compression mode
    with zipfile.ZipFile(output_file_path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zip_file:
        # Add the input file to the zip file
        zip_file.write(input_file_path, input_file)

    # Print a message indicating that the file has been zipped
    print(f"{input_file} has been zipped to {output_file_name}")
    
