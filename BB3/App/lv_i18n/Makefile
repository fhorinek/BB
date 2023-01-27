LV_I18N = ../../../Utilities/lv_i18n/node_modules/.bin/lv_i18n

help:
	@echo 'Use "make extract" to extract new strings for translation'
	@echo 'Use "make compile" to compile translations into source code'

extract:
	$(LV_I18N) extract -s '../**/*.+(c|cpp|h|hpp|inc)' -t '*.yml' --dump-sourceref sourceref.json --dump-sourceref-url "https://github.com/fhorinek/BB/tree/master/BB3/App/lv_i18n/"

compile:
    #en-US must be the last one !!!
	$(LV_I18N) compile -l sk-SK -l de-DE -l fr-FR -l pl-PL -l en-US -t '*.yml' --raw lv_i18n_data.inc
