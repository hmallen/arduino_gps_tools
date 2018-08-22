import logging
import os
import sys

logging.basicConfig()
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

# 2018-08-22T00:12:45Z,8,1.09,33.019699,-97.068848,40,592.85,0.00,0.06
csv_columns = ['datetime', 'satellites', 'hdop',
               'latitude', 'longitude', 'age',
               'altitude', 'course', 'speed']


if __name__ == '__main__':
    pass
