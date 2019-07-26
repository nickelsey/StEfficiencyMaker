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

def main(args):

  ## check xml file exists
  xml_file = os.path.join(os.getcwd(), args.submitscript)
  if not os.path.isfile(xml_file) :
    print('xmlfile doesnt exist!')
    return

  ## create output from input variables
  param_string = "geantid_{}_dca_{}_nhit_{}_nhitfrac_{}".format(args.geantid, args.dca, args.nhit, args.nhitfrac)
  out_base = os.path.join(args.outputroot, args.production, args.outputtag, param_string)
  log_dir = os.path.join(out_base, "log")
  out_dir = os.path.join(out_base, "out")

  ## create log and output directories
  if not os.path.exists(log_dir):
    os.makedirs(log_dir)
  if not os.path.exists(out_dir):
    os.makedirs(out_dir)

  ## get the list of input files
  filelist = listAllFiles(args.listdir)

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
  mu_list.sort()
  mc_list.sort()
  
  if len(mc_list) != len(mu_list):
    print("Error: different number of mu and minimc list files")

  print('Submitting jobs - parameters')
  print('number of jobs: ', len(mc_list))
  print('library: ', args.library)
  print('DCA: ', args.dca)
  print('nhit: ', args.nhit)
  print('nhitposs: ', args.nhitfrac)
  print('geant ID', args.geantid)
  print('log directory: ', log_dir)
  print('output directory: ', log_dir)

  for i in range(len(mu_list)) :
    mu_file = mu_list[i]
    mc_file = mc_list[i]

    print("submitting job: ")
    print("muDst file list: " + mu_file)
    print("minimc file list: " + mc_file)

    submit_args = 'lib=' + args.library
    submit_args = submit_args + ',mulist=' + mu_file
    submit_args = submit_args + ',mclist=' + mc_file
    submit_args = submit_args + ',log=' + log_dir
    submit_args = submit_args + ',out=' + out_dir
    submit_args = submit_args + ',dca=' + str(args.dca)
    submit_args = submit_args + ',nhit=' + str(args.nhit)
    submit_args = submit_args + ',nhitfrac=' + str(args.nhitfrac)
    submit_args = submit_args + ',geantid=' + str(args.geantid)

    star_submit = 'star-submit-template '
    star_submit = star_submit + '-template ' + xml_file
    star_submit = star_submit + ' -entities ' + submit_args
    
    print('submit command: ', star_submit)

    ret = subprocess.Popen(star_submit, shell=True)
    ret.wait()
    if ret.returncode != 0:
      print('warning: job submission failure')
    
if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Submit QA jobs to generate efficiency qa', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('--submitscript', default='submit/submit_gen_efficiency.xml', help='the xml file for star-submit-template')
  parser.add_argument('--listdir', default='list', help='directory to lists of mu and mc lists')
  parser.add_argument('--production', default='P18ih', help='production tag associated with the data embedded into')
  parser.add_argument('--outputtag', default='efficiency/AuAu_200_production_2014', help='output directory name (appended to outputroot/production')
  parser.add_argument('--library', default='pro', help='STAR library version to run analysis with')
  parser.add_argument('--outputroot', default='/gpfs01/star/pwg/nelsey', help='root directory for all output and logs')
  parser.add_argument('--dca', default='3.0', help='dca cut for reconstructed tracks')
  parser.add_argument('--nhit', default='15', help='number of reconstructed hits in track reco')
  parser.add_argument('--nhitfrac', default='0.52', help='fraction of reconstructed hits out of possible hits in track reco')
  parser.add_argument('--geantid', default='8', help='geant ID to accept for mc tracks')
  args = parser.parse_args()
  main( args )

