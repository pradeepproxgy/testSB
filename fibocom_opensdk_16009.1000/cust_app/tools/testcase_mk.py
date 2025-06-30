import os
import sys
import re
from pathlib import Path
import shutil
from shutil import copyfile
from fnmatch import fnmatch


structstr = """
typedef struct
{
    int idx;
    const char *name;
    void (*func)();
} testcase_t;

typedef struct
{
    int idx;
    const char *name;
    void (*init)();
    void (*deinit)();
    testcase_t *filecase;
    int filecasecnt;
} testcase_file_t;

typedef int (*testcase_init_func_t)();
typedef int (*testcase_deinit_func_t)();
"""
alltestcase = []
allinitfunc = []
alldeintfunc = []

def parse_testcase_file(filepath):
    filebasename = os.path.basename(filepath)
    testcasename = os.path.splitext(filebasename)[0]
    filecasees = {"name": testcasename, "enter": None, "exit": None, "case": []}
    with open(filepath, "r") as fp:
       for line in fp.readlines():
           
           result = re.search("void\s+testcase_(\w+)_enter\(\w*\)", line)
           if result is not None:
               filecasees['enter'] = result[1]
               continue

           result = re.search("void\s+testcase_(\w+)_exit\(\w*\)", line)
           if result is not None:
               filecasees['exit'] = result[1]
               continue

           result = re.search("void\s+testcase_(\w+)\(", line)
           if result is not None:
               filecasees['case'].append(result[1]) 

           result = re.search("int\s+testcase_(\w+)_init\(", line)
           if result is not None:
               allinitfunc.append(result[1])

           result = re.search("int\s+testcase_(\w+)_deinit\(", line)
           if result is not None:
               alldeintfunc.append(result[1])
    alltestcase.append(filecasees)

def main(casepath, outpath):
    for root, _, files in os.walk(casepath):
        for file_name in files:
            if len(os.path.splitext(file_name)) > 1:
                extname = os.path.splitext(file_name)[1]
                if extname in ['.c']:
                    file_path = os.path.join(root, file_name)
                    print(file_path)
                    parse_testcase_file(file_path)


    with open(outpath, "w") as fp:
        fp.write(structstr)
        fp.write("\n\n")

        for index, item in enumerate(allinitfunc):
            fp.write("int testcase_%s_init();" % (item))
            fp.write("\n")

        for index, item in enumerate(alldeintfunc):
            fp.write("int testcase_%s_deinit();" % (item))
            fp.write("\n")

        for index, item in enumerate(alltestcase):
            if item['enter'] is not None:
                fp.write("void testcase_%s_enter();" % (item['enter']))
                fp.write("\n")

            if item['exit'] is not None:
                fp.write("void testcase_%s_exit();" % (item['exit']))
                fp.write("\n")

            for index1, item1 in enumerate(item['case']):
                fp.write("void testcase_%s();" % (item1))
                fp.write("\n")

        fp.write("\n\n")

        for index, item in enumerate(alltestcase):
            fp.write("static testcase_t g_file_%s_testcase[] = {" % (item['name']))
            for index1, item1 in enumerate(item['case']):
                fp.write("{%d, \"%s\", testcase_%s}" % (index1, item1, item1))
                if index1 != (len(item['case']) - 1):
                    fp.write(",")
            fp.write("};")
            fp.write("\n\n")  
        
        fp.write("static testcase_file_t g_alltestcase[] = {")
        for index, item in enumerate(alltestcase):
            enterfunc = "NULL" if item['enter'] is None else "testcase_%s_enter"  % (item['enter']) 
            exitfunc = "NULL" if item['exit'] is None else "testcase_%s_exit"  % (item['exit'])
            filecase = "g_file_%s_testcase" % (item['name'])
            fp.write("{%d, \"%s\", %s, %s, %s, sizeof(%s)/sizeof(%s[0])}" % (index, item['name'], enterfunc , exitfunc, filecase, filecase, filecase))
            if index != (len(alltestcase) - 1):
                fp.write(",")
        fp.write("};")
        fp.write("\n\n")

        fp.write("static testcase_init_func_t g_alltestcaseinit[] = {")
        for index, item in enumerate(allinitfunc):
            fp.write("testcase_%s_init" % (item))
            if index != (len(allinitfunc) - 1):
                fp.write(",")
        fp.write("};")
        fp.write("\n\n")

        # fp.write("static testcase_deinit_func_t g_alltestcasedeinit[] = {")
        # for index, item in enumerate(alldeintfunc):
        #     fp.write("testcase_%s_deinit" % (item))
        #     if index != (len(alldeintfunc) - 1):
        #         fp.write(",")
        # fp.write("};")
        # fp.write("\n\n")

main(sys.argv[1], sys.argv[2])