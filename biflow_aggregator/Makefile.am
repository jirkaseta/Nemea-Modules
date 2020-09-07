ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS=biflow_aggregator
biflow_aggregator_SOURCES=main.cpp aggregator.cpp fields.c configuration.cpp xxhash.c key_template.cpp
biflow_aggregator_LDADD=-lunirec -ltrap
biflow_aggregator_CXXFLAGS=-Wall -std=gnu++11 -O3
include ../aminclude.am
