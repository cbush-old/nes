#!/bin/sh -e
for i in *.nes; do
  MAPPER=`get_mapper "$i"`
  DIR="Mapper$MAPPER"
  mkdir -p "$DIR"
  cp "$i" "$DIR"
  echo "$i" "->" "$DIR"
done
