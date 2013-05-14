#!/usr/bin/env python

import sys, os, os.path
import re
import datetime
import time
import random
import hashlib
import base64

OUT_C = '1_2_Conways_game_of_life.cpp'
OUT_H = '1_2_Conways_game_of_life.h'

DEBUG=2

# Files:
out_c = None
out_h = None

# Files to process:
C_FILES=[]

# Headers to process:
H_FILES=[]

# Header names:
MY_HEADERS={}

# Headers, alway included:
INC_HEADERS = {}

def rndname(id, filename):
    md5 = hashlib.md5()
    md5.update(id)
    md5.update(filename)
    md5.update(time.ctime())
    md5.update(str(random.randrange(1000000)))

    h = md5.digest()

    # Create identifier:
    return "S_%s" % base64.b32encode(h).replace('=', '_')

r_inc = re.compile('^\\s*#\\s*include\\s+[<"]([-a-zA-Z0-9_\\.\\/]+)[>"]')
r_static = re.compile('(static([\\s\n\\*]+[_a-zA-Z][_a-zA-Z0-9]*)+)[\\s\n]*[=;\\(\\[]', re.MULTILINE)
r_struct = re.compile('(struct[\\s\n]+[_a-zA-Z][_a-zA-Z0-9]*)[\\s\n]*\\{')
IGNORE_STATICS = [
    "timegm_r",
    "gmtime_r",
    "timegm",
    "localtime_r"
]

def process_h_file(filename):
    bn = os.path.basename(filename)
    if bn in INC_HEADERS:
        return # Always processed

    INC_HEADERS[bn] = 1 # Prevent recursion

    if DEBUG:
        print "Processing %s..." % bn

    out_h.write("/* %s */\n" % bn)
    f = open(filename, "rt")
    for l in f:
        m = r_inc.match(l)
        if m:
            # This is include:
            inc_nm = m.group(1)
            i_nm = os.path.basename(inc_nm)
            if i_nm in MY_HEADERS:
                if i_nm not in INC_HEADERS:
                    process_h_file(MY_HEADERS[i_nm])
            else:
                out_h.write(l)
        else:
            out_h.write(l)
    f.close()

def process_c_file(filename):
    bn = os.path.basename(filename)

    if DEBUG:
        print "Processing %s..." % bn

    out_c.write("/* %s */\n" % bn)

    # 1. Search all statics:
    f = open(filename, "rt")
    content = f.read()
    f.close()

    statics = r_static.findall(content)

    static_ids = []
    for m in statics:
        # Name is last element in group:
        l_ids = m[0].split() # identifiers
        l_ids = re.split('[\\s\\*]+', m[0])
        l_id = l_ids[-1]
        if l_id not in static_ids and l_id not in IGNORE_STATICS:
            static_ids.append(l_id)
    # Process also local structures definitions:
    structs = r_struct.findall(content)
    for s in structs:
        nm = s.split()[-1]
        if nm not in static_ids and nm not in IGNORE_STATICS:
            static_ids.append(nm)

    del content
    if static_ids != []:
        # Define all of them to random identifiers:
        out_c.write("""
        /* static variables for %s */
        %s
        """ % (filename, "\n".join(["#define %s %s" % (n, rndname(n, filename)) for n in static_ids])))

    f = open(filename, "rt")
    for l in f:
        m = r_inc.match(l)
        if m:
            inc_nm = m.group(1)
            i_nm = os.path.basename(inc_nm)
            if i_nm in MY_HEADERS:
                if l.find('/*') < 0 and l.find('*/') < 0:
                    out_c.write("/* %s */" % l)
            else:
                out_c.write(l)
        else:
            out_c.write(l)
    if static_ids != []:
        out_c.write("""
        /* undef static variables for %s */
        %s
        """ % (filename, "\n".join(["#undef %s" % n for n in static_ids])))
    else:
        out_c.write("\n")
    f.close()

def scandir(nm):
    " Scan directory and subrirectories for .c, .cpp and .h files "
    global C_FILES, H_FILES
    files = os.listdir(nm)
    for f in files:
        curnm = os.path.join(nm, f)
        if f.endswith(".c") or f.endswith(".cpp"):
            C_FILES.append(curnm)
        elif f.endswith(".h"):
            H_FILES.append(curnm)
            MY_HEADERS[f] = curnm
        else:
            if os.path.isdir(curnm):
                scandir(curnm)

def filesfrommakefile(nm):
    " prepare lists from makefile "
    global C_FILES, H_FILES
    MKV = { 'ASN1CPREFIX': os.path.dirname(nm) }
    f = open(nm, 'rt')
    line = ""
    mk_var = re.compile('\\$\\([^\\(\\)]*\\)|\\${[^{}]}|\\$[a-zA-Z0-9_]')

    def replfcn(m):
        v = m.group(0)[1:]
        if v.startswith('(') or v.startswith('{'):
            v = v[1:-1]
        
        if v in MKV:
            return MKV[v]
        else:
            return ""

    for l in f:
        if len(line) > 0:
            line = line + l
        else:
            line = l
        if line.endswith('\\\n'):
            line = line[:-2] + ' '
        else:
            # Processing line:
            ln = mk_var.sub(replfcn, line)

            if ln.find('+=') > 0: # Add value to variable
                nm, val = ln.split('+=')
                nm = nm.strip()
                if nm not in MKV:
                    MKV[nm] = val
                else:
                    MKV[nm] = MKV[nm] + val
            elif ln.find('=') > 0: # Set value of variable
                nm, val = ln.split('=')
                nm = nm.strip()
                MKV[nm] = val

            line = ""

    if 'ASN1C_H' not in MKV:
        print >> sys.stderr, "Error: can not find ASN1C_H in makefile"
        sys.exit(1)

    if 'ASN1C_SOURCES' not in MKV:
        print >> sys.stderr, "Error: can not find ASN1C_SOURCES in makefile"
        sys.exit(1)

    hs = MKV['ASN1C_H'].split()
    cs = MKV['ASN1C_SOURCES'].split()

    for c in cs:
        C_FILES.append(c)
    print hs
    print cs
    for h in hs:
        H_FILES.append(h)
        MY_HEADERS[os.path.basename(h)] = h

def main():
    global out_c, out_h
    # Command line arguments:
    if len(sys.argv) != 2:
        print "Usage: %s asn1c-sourcedir-or-makefile" % sys.argv[0]
        sys.exit(1)
    dirnm = sys.argv[1]

    if os.path.isdir(dirnm):
        # Scan directory for files (recursive):
        scandir(dirnm)
    else:
        filesfrommakefile(dirnm)

    out_c = open(OUT_C, 'wt')
    out_h = open(OUT_H, 'wt')

    # Generate header:
    t = "ASN1C_LOCAL_H___%d" % time.time()
    out_h.write("""#ifndef %s
#define %s 1

/* This file is automatically generated from asn1c sources */
/* If you want change something - edit original files, not me */

""" % (t, t))

    for h in H_FILES:
        process_h_file(h)

    out_h.write("""
#endif /* %s */

/* End of automatically generated file */
""" % t)
    out_c.write("""/* This is automatically generated file */
/* All asn1c includes provided by one file. We will not duplicate include of */
#include "asn1c_local.h"

""")
    for c in C_FILES:
        process_c_file(c)

    out_h.close()
    out_c.close()

if __name__ == '__main__':
    main()