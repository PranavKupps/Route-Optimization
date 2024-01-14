import os
import googlemaps
from datetime import datetime

gmaps = googlemaps.Client(key='Add Your Key here')

string filename = 'INSERT FILENAME.txt'


olympicsfile = open(filename, "r")

for aline in olympicsfile:
    geocode_result = gmaps.geocode(aline)
    print(geocode_result, "\n")

olympicsfile.close()




# Geocoding an address
geocode_result = gmaps.geocode('1600 Amphitheatre Parkway, Mountain View, CA')
