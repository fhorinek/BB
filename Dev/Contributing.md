# Contributing

We welcome any contribution from our users or fans. 

We are using 3 types of firmware:
 * devel
 * testing 
 * release

If you want to develop feature for the strato checkout the `master` branch and name it accordingly
`type` / `description`

good examples: 
* fix/glider_ratio_decimal_points
* widget/time_date
* feature/csv_logger
* utility/map_simulator

## Firmware versions
To fully accomodate 3 flavors of the firmware we are using 3 numbers to describe version.

`build_number.testing_number.release_number`

build_number is incremented each time new devel firmware is emmited and it serve as reference to master branch.
testing_number is incremented each time testing firmware is emmited (fix applied).
release_number is incremented each time release firmware is emmited (fix applied).


## Branch workflow

Any code additions are made to the `master` branch via PR only (except for branch house keeping).

To emit Testing firmware the `pack_fw.py` with channel set to testing is runned to create firmware bundle and new branch (testing/XXX.x.x, where XXX is build_number).
Testing version is allowed only to receive bug fixes, which are based from its branch and then backported to the main.

When the testing version is stable enought then the `pack_fw.py` is runned to create release version and new branch (relese/XXX.AAA.x, where XXX is build_number and AAA is testing_number)..
From now on only release version can be emmited from this branch, only serious problem are fixed.

Any feature addition must be made to the master first!
Testing and Release recieve only bug fixes.


