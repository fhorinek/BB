import json
import requests

def parse_json(url, output_file):
    # Make a request to the URL and retrieve the JSON data
    response = requests.get(url)
    data = response.json()

    # Open the output file in write mode
    with open(output_file, "w") as f:
        # Iterate over the list of dictionaries and write the "countryname" and "countryid" to the output file
        for dictionary in data:
            line = dictionary["countryname"] + "\t" + str(dictionary["countryid"]) + "\n"
            f.write(line)
        
parse_json("https://airspace.xcontest.org/web/country", "../../BB3/Assets/xcontest_id.db")
