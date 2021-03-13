Strato
=====

This is the source code repository for Strato, a paragliding vario
manufactured by SkyBean.

<b> !!! If you are looking for updates go to this site (or download files from github as RAW) !!!</b>
[http://vps.skybean.eu/repo/updates/](http://vps.skybean.eu/repo/updates/)

I want to ...
* [... update SkyDrop firmware](http://skybean.eu/support)
* [... configure SkyDrop via configurator](http://vps.skybean.eu/configurator/)
* [... report bug/suggest feature/ask for help](https://github.com/fhorinek/SkyDrop/issues/new)
* [... see what features are implemented in lastest release](https://github.com/fhorinek/SkyDrop/blob/master/updates/changelog.txt)
* [... look for solution to my problem](https://github.com/fhorinek/SkyDrop/issues?utf8=%E2%9C%93&q=is%3Aissue+label%3Aquestion)
* [... see what features will be implemented in next release](https://github.com/fhorinek/SkyDrop/milestones)
* [... browse issues](https://github.com/fhorinek/SkyDrop/issues)

Bug reporting etiquette
=====
* Search issues before posting new bug report/feature request
* Post Debug.log with the problem (in Debug first Enable debug.log, Clear the log and then recreate the issue)
* Tell us what was the trigger of the bug (opening menu, landing, certain settings)

Folder structure
=====

<b>BB2</b><br>
Todo: Please descibe what this folder contains<br>
<b>BB2_loader</b><br>
Todo: Please descibe what this folder contains<br>
<b>BB3</b><br>
Todo: Please descibe what this folder contains<br>
<b>BB3_loader</b><br>
Todo: Please descibe what this folder contains<br>
<b>BB_esp_fw</b><br>
Todo: Please descibe what this folder contains<br>
<b>BB_map_sim</b><br>
Todo: Please descibe what this folder contains<br>
<b>Utilities</b><br>
Todo: Please descibe what this folder contains<br>

External library and code
=====

We are standing on shoulders these giants:

<b>SkyDrop</b>

Tools info
=====

Tools we are using:

<br>
STM32CubeIDE v1.5.1 from https://www.st.com/en/development-tools/stm32cubeide.html<br>

Build info
=====

<ul>
<li>Download and Install STM32CubeIDE</li>
<li>cd $HOME ; git clone https://github.com/fhorinek/BB.git</li>
<li>Start STM32CubeIDE with /opt/st/stm32cubeide_1.5.1/stm32cubeide</li>
<li>Choose an arbitraty workspace location</li>
<li>Select "Start new project from an existing STM32CubeMX configuration file (.ioc)"</li>
<li>Select STM32CubeMX .ioc file $HOME/BB/BB3/BB3.ioc and select Finish
</li>
</ul>
