#!/bin/bash

# author: Luxamnis#2153

# --------------------------------
# KKO testing script
# --------------------------------

make || exit

DATA_DIR="data"
COM_DATA="com_data"
COM_M_DATA="com_m_data"
DEC_DATA="dec_data"
DEC_M_DATA="dec_m_data"

# 1) Create directories
[ ! -d $COM_DATA ] && mkdir $COM_DATA
[ ! -d $COM_M_DATA ] && mkdir $COM_M_DATA
[ ! -d $DEC_DATA ] && mkdir $DEC_DATA
[ ! -d $DEC_M_DATA ] && mkdir $DEC_M_DATA

# 2) Clean directories if necessary
rm ${COM_DATA:?}/*
rm ${COM_M_DATA:?}/*
rm ${DEC_DATA:?}/*
rm ${DEC_M_DATA:?}/*

# 3) Run compression and decompression, measure compression time
for DATAFILE in ${DATA_DIR}/*.raw
do
    FILENAME=$(basename $DATAFILE)
    # compression
    echo "a) Compressing (sequential no model) ${DATAFILE} ..."
    /usr/bin/time -f "%e" ./huff_codec -c -i ${DATAFILE} -o "${COM_DATA}/${FILENAME}.com"
    echo "b) Compressing (sequential with model) ${DATAFILE} ..."
    /usr/bin/time -f "%e" ./huff_codec -c -m -i "${DATAFILE}" -o "${COM_M_DATA}/${FILENAME}.comm"
    echo "c) Compressing (adaptive no model) ${DATAFILE} ..."
    /usr/bin/time -f "%e" ./huff_codec -c -a -i ${DATAFILE} -o "${COM_DATA}/${FILENAME}.coma" -w 512
    echo "d) Compressing (adaptive with model) ${DATAFILE} ..."
    /usr/bin/time -f "%e" ./huff_codec -c -a -m -i "${DATAFILE}" -o "${COM_M_DATA}/${FILENAME}.comam" -w 512

    # decompression
    echo "a) Decompressing (no model) ${DATAFILE} ..."
    ./huff_codec -d -i "${COM_DATA}/${FILENAME}.com" -o "${DEC_DATA}/${FILENAME}.dec"
    echo "b) Decompressing (with model) ${DATAFILE} ..."
    ./huff_codec -d -m -i "${COM_M_DATA}/${FILENAME}.comm" -o "${DEC_M_DATA}/${FILENAME}.decm"
    echo "c) Decompressing (adaptive no model) ${DATAFILE} ..."
    ./huff_codec -d -a -i "${COM_DATA}/${FILENAME}.coma" -o "${DEC_DATA}/${FILENAME}.deca" -w 512
    echo "d) Decompressing (adaptive with model) ${DATAFILE} ..."
    ./huff_codec -d -a -m -i "${COM_M_DATA}/${FILENAME}.comam" -o "${DEC_M_DATA}/${FILENAME}.decam" -w 512
done

# 4) run difference tests
echo ""
echo "Difference tests:"
for SRC_FILE in ${DATA_DIR}/*.raw
do
    SRC_FILE_BASE=$(basename $SRC_FILE)
    echo "  Dirrerence for: ${SRC_FILE_BASE}:"
    diff -s -q $SRC_FILE ${DEC_M_DATA}/${SRC_FILE_BASE}.decm
    diff -s -q $SRC_FILE ${DEC_DATA}/${SRC_FILE_BASE}.dec
    diff -s -q $SRC_FILE ${DEC_M_DATA}/${SRC_FILE_BASE}.decam
    diff -s -q $SRC_FILE ${DEC_DATA}/${SRC_FILE_BASE}.deca
done

# 5) Measure compression efficiency
echo ""
echo "Compression efficiency:"
for SRC_FILE in ${DATA_DIR}/*.raw
do
    SRC_FILE_BASE=$(basename $SRC_FILE)

    echo ""
    echo "  Efficiency for: ${SRC_FILE_BASE}:"
    ORIG_SIZE=$(du -hb ${SRC_FILE} | cut -f 1)
    COM_SIZE=$(du -hb ${COM_DATA}/${SRC_FILE_BASE}.com | cut -f 1)
    EFFICIENCY=$(echo "scale=2; (${COM_SIZE}*8)/${ORIG_SIZE}" | bc)
    echo "Sequential no model: ${EFFICIENCY}"

    ORIG_SIZE=$(du -hb ${SRC_FILE} | cut -f 1)
    COM_SIZE=$(du -hb ${COM_M_DATA}/${SRC_FILE_BASE}.comm | cut -f 1)
    EFFICIENCY=$(echo "scale=2; (${COM_SIZE}*8)/${ORIG_SIZE}" | bc)
    echo "Sequential with model: ${EFFICIENCY}"

    ORIG_SIZE=$(du -hb ${SRC_FILE} | cut -f 1)
    COM_SIZE=$(du -hb ${COM_DATA}/${SRC_FILE_BASE}.coma | cut -f 1)
    EFFICIENCY=$(echo "scale=2; (${COM_SIZE}*8)/${ORIG_SIZE}" | bc)
    echo "Adaptive no model: ${EFFICIENCY}"

    ORIG_SIZE=$(du -hb ${SRC_FILE} | cut -f 1)
    COM_SIZE=$(du -hb ${COM_M_DATA}/${SRC_FILE_BASE}.comam | cut -f 1)
    EFFICIENCY=$(echo "scale=2; (${COM_SIZE}*8)/${ORIG_SIZE}" | bc)
    echo "Adaptive with model: ${EFFICIENCY}"


done
