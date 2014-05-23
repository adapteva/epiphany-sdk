#!/usr/bin/env bash

printf "Starting the Epiphany SDK build...\n"
if ! ./build-epiphany-sdk.sh; then
	printf "Epiphany SDK build completed with errors\n"
else
	printf "Epiphany SDK build completed successfully\n"
fi


