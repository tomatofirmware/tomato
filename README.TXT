# Tomato RAF is a forked version of Tomato firmware conceived by Jonathan Zarate, 2007.
# 
#
# Copyright 2001-2003, Broadcom Corporation
# All Rights Reserved.
#
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
#

There are multiple directories provided here. The release directory
contains the router source package. The uClibc directory contains the source
for uClibc. The binary tools required to build are provided as a
convenience.

To build the router:
 o Follow the instructions in the tools/ directory to install the toolchain
 o Ensure the tools in the release/tools directory are in your path
 o cd release/src-rt
 o type 'make help' to see the options to build for each model

This will create a parallel directory to src-rt called image that contains the
final image.

Base Image for Tomato-RAF-RT-N build 9013, v1.2i
Sept 30th, 2013
