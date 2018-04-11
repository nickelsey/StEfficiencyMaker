#!/bin/csh

# small macro to clean up output space

set SCRATCHSPACE PATH/TO/SCRATCH
rm -rv ${SCRATCHSPACE}/tmplogs/*

set word = "tmp"
while ($word != "")
   echo -n "DELETE [logs/output] (Return to exit): "
   set word = $<
   if ( $word == 'output' ) rm -rv ${SCRATCHSPACE}/out/*
   if ( $word == 'logs' ) rm -rv ${SCRATCHSPACE}/logs/log/*
   if ( $word == 'logs' ) rm -rv ${SCRATCHSPACE}/logs/err/*
end

