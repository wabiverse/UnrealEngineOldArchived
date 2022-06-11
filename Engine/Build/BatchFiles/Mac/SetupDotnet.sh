#!/bin/bash
# Copyright Epic Games, Inc. All Rights Reserved.

START_DIR=`pwd`
cd "$1"

UE_USE_SYSTEM_DOTNET=1
IS_DOTNET_INSTALLED=0
DOTNET_VERSION_PATH=$(command -v dotnet) || true

if [ "$UE_USE_SYSTEM_DOTNET" == "1" ] && [ ! $DOTNET_VERSION_PATH == "" ] && [ -f $DOTNET_VERSION_PATH ]; then
	# If dotnet is installed, check that it has a new enough version of the SDK
	DOTNET_SDKS=(`dotnet --list-sdks | ggrep -P "(\d*)\.(\d*)\..* \[(.*)\]"`)
	for DOTNET_SDK in $DOTNET_SDKS
	do
		if [ "$DOTNET_SDK" == "6.0.300" ]; then
			IS_DOTNET_INSTALLED=1
		fi
	done
	if [ $IS_DOTNET_INSTALLED -eq 0 ]; then
	echo Unable to find installed dotnet sdk of version 3.1 or newer
	fi
fi

# Setup bundled Dotnet if cannot use installed one
if [ $IS_DOTNET_INSTALLED -eq 0 ]; then
	echo Setting up bundled DotNet SDK
	CUR_DIR=`pwd`

	# Select the preferred architecture for the current system
	ARCH=x64
	[ $(uname -m) == "arm64" ] && ARCH=arm64 
	
	export UE_DOTNET_DIR=$CUR_DIR/../../../Binaries/ThirdParty/DotNet/6.0.200/mac-$ARCH
	echo $UE_DOTNET_DIR
	export PATH=$UE_DOTNET_DIR:$PATH
	export DOTNET_ROOT=$UE_DOTNET_DIR
else
	export IS_DOTNET_INSTALLED=$IS_DOTNET_INSTALLED
fi

cd "$START_DIR"
