This directory contains all texts from the application together with
translations to different languages. We use https://github.com/lvgl/lv_i18n
to manage this. Please follow this side for more information.

# For translators

Please look at de-DE.yml or other files for missing translations and
translate them. Please do a pull request on github.com to request
inclusion of your work.

# For developers

Whenever you introduce a new string which should be translated, then please
use ```_()``` to declare them for translation. E.g. 

```c
printf("Hello World\n");
```

should be changed to

```c
printf(_("Hello World\n"));
```

Please use english language as the base language.

You can run the program without having proper translations. It will fall back to english, exactly how you wrote your string.

When you finished your functionality, then please go to BB3/lv_i18n and execute ```"make extract"``` to extract your new strings for translation. They will be added to the yml files in this directory and are then available for translators. In order to do this, please follow the instructions in https://github.com/lvgl/lv_i18n#readme how to install "lv_i18n" on your system. We recomment to install it globally with ```"npm i littlevgl/lv_i18n -g"```.

Optionally follow the maintainer step, if you think it makes sense.

# For maintainer (fhorinek and others)

Whenever you receive a PR, then do a ```"make compile"``` to compile the translations into source code, which is then included into the application.

