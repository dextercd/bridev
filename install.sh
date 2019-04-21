#!/bin/sh

destination="$HOME/bin/bridev"
sudo cp bridev "$destination"
sudo chown root "$destination"
sudo chmod a+s "$destination"
