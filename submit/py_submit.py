from __future__ import print_function

from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
import os
import argparse
import re
import subprocess
import time

def listAllFiles(directory):
  files = []
  for dirpath,_,filenames in os.walk(directory):
    for f in filenames:
      files.append(os.path.abspath(os.path.join(dirpath, f)))
  return files

def update_submit_script(file_path, mulist, mclist, geantid, dca, nhit, nhitpos, rootdir, outdir, library):
  #Create temp file
  tmpfile = 'tmp_submit.xml'
  abs_path = os.path.join(os.getcwd(), tmpfile)
  with open(tmpfile,'w') as new_file:
    with open(file_path) as old_file:
      for line in old_file:
        if line.find("ENTITY MUFILE") != -1 :
          new_file.write("<!ENTITY MUFILE \"" + str(mulist) + "\">\n")
        elif line.find("ENTITY MCFILE") != -1 :
          new_file.write("<!ENTITY MCFILE \"" + str(mclist) + "\">\n")
        elif line.find("ENTITY GEANTID") != -1 :
          new_file.write("<!ENTITY GEANTID \"" + str(geantid) + "\">\n")
        elif line.find("ENTITY DCA") != -1 :
          new_file.write("<!ENTITY DCA \"" + str(dca) + "\">\n")
        elif line.find("ENTITY ROOTDIR") != -1 :
          new_file.write("<!ENTITY ROOTDIR \"" + str(rootdir) + "\">\n")
        elif line.find("ENTITY OUTDIR") != -1 :
          new_file.write("<!ENTITY OUTDIR \"" + str(outdir) + "\">\n")
        elif line.find("ENTITY PROD") != -1 :
          new_file.write("<!ENTITY PROD \"" + str(library) + "\">\n")
        elif line.find("ENTITY NHITPOS") != -1 :
          new_file.write("<!ENTITY NHITPOS \"" + str(nhitpos) + "\">\n")
        elif line.find("ENTITY NHIT") != -1 :
          new_file.write("<!ENTITY NHIT \"" + str(nhit) + "\">\n")
        else :
          new_file.write(line)
    #Remove original file
    remove(file_path)
    #Move new file
    move(abs_path, file_path)


def main(args):
  
  ## create output directory
  out_directory = 'out' + args.geantid
  if not os.path.exists(out_directory):
    os.makedirs(out_directory)
    logdir = out_directory + '/tmplogs'
    os.makedirs(logdir)

  xml_file = os.path.join(os.getcwd(), args.submitscript)
  if not os.path.isfile(xml_file) :
    print('xmlfile doesnt exist!')
    return

  ## get our absolute path and define submission variables
  rootdir = os.getcwd()
  outdir = os.path.join(rootdir, out_directory)

  ## now get the list of input files
  filelist = listAllFiles(args.listDir)

  ## sort the files into mu and mc lists
  find_mu = re.compile('mu\d+.list')
  mu_list = []
  find_mc = re.compile('mc\d+.list')
  mc_list = []
  for file in filelist :  
    if find_mu.search(file) :
      mu_list.append(file)
    elif find_mc.search(file) :
      mc_list.append(file)

  ## they must be the same length or else something is messed up
  if len(mc_list) != len(mu_list) :
    print("mu file list length does not match mc file list length!")
    return
  ## now sort them
  mu_list.sort()
  mc_list.sort()

  ## and we can start job submission now
  processes = []
  for i in range(len(mu_list)) :
    mu_file = mu_list[i]
    mc_file = mc_list[i]
    update_submit_script(xml_file, mu_file, mc_file, args.geantid, args.dca, args.nhits, args.nhitspos, rootdir, outdir, args.library)

    star_submit = 'star-submit ' + xml_file
    print("submitting job: ")
    print("muDst file list: " + mu_file)
    print("minimc file list: " + mc_file)
    print("geant ID: " + args.geantid)
    print("dca: " + args.dca)
    print("root directory: " + rootdir)
    print("output directory: " + outdir)
    print(star_submit)
    ret = subprocess.Popen(star_submit, shell=True)
    processes.append(ret)
    time.sleep(5)
    #ret.wait()
    #if ret.returncode != 0 :
      #print('warning, job submission failure')
  for i in range(len(processes)):
    proc = processes[i]
    proc.wait()
    if proc.returncode != 0:
      print('warning: job #', i, "failed submission")


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Submit embedding jobs via star-submit')
  parser.add_argument('--listDir', default='minimc_list/piplus', help='location of embedding file lists')
  parser.add_argument('--dca', default='3.0', help='dca cut for reconstructed tracks')
  parser.add_argument('--nhits', default='20', help='number of reconstructed hits in track reco')
  parser.add_argument('--nhitspos', default='0.52', help='fraction of reconstructed hits out of possible hits in track reco')
  parser.add_argument('--geantid', default='8', help='geant id for embedded tracks (default = 8 = pi+)')
  parser.add_argument('--library', default='SL17d_embed', help='star library version')
  parser.add_argument('--submitscript', default='jobSubmit.xml', help='the xml file for star-submit')
  args = parser.parse_args()
  main( args )
