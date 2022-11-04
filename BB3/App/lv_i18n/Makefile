LV_I18N = ../../../Utilities/lv_i18n/node_modules/.bin/lv_i18n

help:
	@echo 'Use "make extract" to extract new strings for translation'
	@echo 'Use "make compile" to compile translations into source code'

extract:
	$(LV_I18N) extract -s '../**/*.+(c|cpp|h|hpp)' -t '*.yml'

compile:
	$(LV_I18N) compile -l sk-SK -l de-DE -t '*.yml' -o '.'
	python3 extract_data.py
