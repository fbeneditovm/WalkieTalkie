#!/bin/bash

ipcs -ma | awk '/^m / { if ($9 == 0) { print $2 }}' | xargs -n 1 ipcrm -m

ipcs -sa | awk '/^s / { print $2 }' | xargs -n 1 ipcrm -s

