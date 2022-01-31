#!/usr/bin/python3

import common

from step1 import pipeline_step1
from step2 import pipeline_step2
from step3 import pipeline_step3
from step4 import pipeline_step4

common.init_vars()
pipeline_step1()
pipeline_step2()
pipeline_step3()
pipeline_step4()

