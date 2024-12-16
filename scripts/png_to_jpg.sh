#!/usr/bin/sh
number=1
for file in *.png; do
    mv "${file}" "${number}.jpg"
    number=$((number + 1))
done