#!/bin/sh

destination="$HOME/bin/bri"
sudo cp bri "$destination"
sudo chown root "$destination"
sudo chmod a+s "$destination"
