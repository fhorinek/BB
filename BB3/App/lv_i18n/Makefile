help:
	@echo 'Use "make extract" to extract new strings for translation'
	@echo 'Use "make compile" to compile translations into source code'

extract:
	lv_i18n extract -s '../**/*.+(c|cpp|h|hpp)' -t '*.yml'

compile:
	lv_i18n compile -l sk-SK -l de-DE -t '*.yml' -o '.'
	python3 extract_data.py
