#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse, glob, os, random, shutil, subprocess, sys, thread, time
import pp

g_dir = 'experiment'
g_plotter = './puddleworld.py'
g_rules = 'PuddleWorld/puddle-world.soar'

g_ep_tuples = []

# ./puddleworld-batch.py -j 4 -r 30 -e 1600
#g_ep_tuples.append(((0, 0, 1, 1), (5, 5), 'even', 'bellman'))
#g_ep_tuples.append(((0, 0, 1, 1), (5, 5), 'even', 'bellman', (0, 10, 10)))
#g_ep_tuples.append(((0, 0, 1, 1), (5, 5), 'fc', 'bellman', (0, 10, 10)))
#g_ep_tuples.append(((0, 0, 1, 1), (5, 5), 'rl', 'bellman', (0, 10, 10)))
#g_ep_tuples.append(((0, 0, 1, 1), (5, 5), 'log-rl', 'bellman', (0, 10, 10)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'even', 'bellman'))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'even', 'bellman', (0, 20, 20)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'fc', 'bellman', (0, 20, 20)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'rl', 'bellman', (0, 20, 20)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'log-rl', 'bellman', (0, 20, 20)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'even', 'bellman', (0, 40, 40)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'fc', 'bellman', (0, 40, 40)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'rl', 'bellman', (0, 40, 40)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'log-rl', 'bellman', (0, 40, 40)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'even', 'bellman', (0, 20, 20), (0, 40, 40)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'fc', 'bellman', (0, 20, 20), (0, 40, 40)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'rl', 'bellman', (0, 20, 20), (0, 40, 40)))
#g_ep_tuples.append(((0, 0, 1, 1), (10, 10), 'log-rl', 'bellman', (0, 20, 20), (0, 40, 40)))

#g_ep_tuples.append(((0.15, .3, .35, .5), (10, 10), 'even', 'bellman', (0, 40, 40)))
#g_ep_tuples.append(((0.15, .3, .35, .5), (10, 10), 'fc', 'bellman', (0, 40, 40)))
#g_ep_tuples.append(((0.15, .3, .35, .5), (10, 10), 'rl', 'bellman', (0, 40, 40)))
#g_ep_tuples.append(((0.15, .3, .35, .5), (10, 10), 'log-rl', 'bellman', (0, 40, 40)))

#g_ep_tuples.append(((0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman'))
#g_ep_tuples.append(((0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman'))
#g_ep_tuples.append(((0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (100, 10, 10)))
#g_ep_tuples.append(((0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman', (400, 20, 20)))
#g_ep_tuples.append(((0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (100, 10, 10), (400, 20, 20)))
#g_ep_tuples.append(((0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman', (400, 20, 20), (800, 40, 40)))
#g_ep_tuples.append(((0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (100, 10, 10), (400, 20, 20), (800, 40, 40)))

#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (0, 10, 10)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (0, 20, 20), (0, 40, 40)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (0, 10, 10), (0, 20, 20), (0, 40, 40)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (200, 20, 20), (200, 40, 40)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (100, 20, 20), (100, 40, 40)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (20, 20), 'rl', 'none', 'bellman'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman', (100, 20, 20)))

#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (20, 20), 'rl', 'none', 'bellman'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman', (0, 20, 20)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman', (100, 20, 20)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (10, 10), 'rl', 'variance', 'bellman', (0, 20, 20)))

#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (0, 10, 10)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (100, 10, 10)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'variance', 'bellman', (0, 10, 10)))

#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (0, 20, 20), (0, 40, 40), (0, 80, 80)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'variance', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (0, 20, 20), (0, 40, 40), (0, 80, 80)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (0, 20, 20), (0, 40, 40), (0, 80, 80), (0, 160, 160)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (0, 20, 20), (0, 40, 40), (0, 80, 80), (0, 160, 160), (0, 320, 320)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (0, 20, 20), (0, 40, 40), (0, 80, 80), (0, 160, 160), (0, 320, 320), (0, 640, 640)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (1.25, 1.25), 'rl', 'none', 'bellman', (0, 2.5, 2.5), (0, 5, 5), (0, 10, 10), (0, 20, 20), (0, 40, 40), (0, 80, 80), (0, 160, 160), (0, 320, 320), (0, 1280, 1280)))

#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'bellman'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (10, 10), 'rl', 'none', 'simple'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'simple'))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'bellman', (0, 10, 10)))
#g_ep_tuples.append((g_rules, (0.15, .15, .45, .45), (5, 5), 'rl', 'none', 'simple', (0, 10, 10)))


## baselines
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (10, 10), 'rl', 'eligibility', 'simple'))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (10, 10), 'rl', 'eligibility', 'simple', (0, 20, 20)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (20, 20), 'rl', 'eligibility', 'simple'))

## approximation of forced splitting down to 16x16 a priori
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (1, 1), 'rl', 'eligibility', 'simple', (0, 1, 1), (0, 1, 1), (0, 2, 1), (0, 2, 2), (0, 4, 2), (0, 4, 4), (0, 8, 4), (0, 8, 8), (0, 16, 8), (0, 16, 16)))


## retest others
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (1, 1), 'even', 'eligibility', 'simple', (0, 1, 1), (0, 1, 1), (0, 2, 1), (0, 2, 2), (0, 4, 2), (0, 4, 4), (0, 8, 4), (0, 8, 8)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (1, 1), 'fc', 'eligibility', 'simple', (0, 1, 1), (0, 1, 1), (0, 2, 1), (0, 2, 2), (0, 4, 2), (0, 4, 4), (0, 8, 4), (0, 8, 8)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (1, 1), 'rl', 'eligibility', 'simple', (0, 1, 1), (0, 1, 1), (0, 2, 1), (0, 2, 2), (0, 4, 2), (0, 4, 4), (0, 8, 4), (0, 8, 8)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (1, 1), 'log-rl', 'eligibility', 'simple', (0, 1, 1), (0, 1, 1), (0, 2, 1), (0, 2, 2), (0, 4, 2), (0, 4, 4), (0, 8, 4), (0, 8, 8)))


## requires learning rate 0.5 and discount rate 0.9
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (5, 5), 'even', 'eligibility', 'simple', (0, 10, 10)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (5, 5), 'fc', 'eligibility', 'simple', (0, 10, 10)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (5, 5), 'rl', 'eligibility', 'simple', (0, 10, 10)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (5, 5), 'log-rl', 'eligibility', 'simple', (0, 10, 10)))

#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (4, 4), 'rl', 'eligibility', 'simple'))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (8, 8), 'rl', 'eligibility', 'simple'))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (16, 16), 'rl', 'eligibility', 'simple'))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (4, 4), 'rl', 'eligibility', 'simple', (0, 8, 8)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (8, 8), 'rl', 'eligibility', 'simple', (0, 16, 16)))
#g_ep_tuples.append((g_rules, (0, 0, 1, 1), (4, 4), 'rl', 'eligibility', 'simple', (0, 8, 8), (0, 16, 16)))


## splitting agent 2x2 on up
#g_ep_tuples.append(('../puddle-world/puddle-world-overgeneral.soar', (0, 0, 1, 1), (0, 0), 'rl', 'eligibility', 'uperf', 0.84155))
#g_ep_tuples.append(('../puddle-world/puddle-world-overgeneral.soar', (0, 0, 1, 1), (0, 0), 'rl', 'eligibility', 'uperf', 0.5))
#g_ep_tuples.append(('../puddle-world/puddle-world-overgeneral.soar', (0, 0, 1, 1), (0, 0), 'rl', 'eligibility', 'td-error', 0.84155))
#g_ep_tuples.append(('../puddle-world/puddle-world-overgeneral.soar', (0, 0, 1, 1), (0, 0), 'rl', 'eligibility', 'td-error', 1))
#g_ep_tuples.append(('../puddle-world/puddle-world-overgeneral.soar', (0, 0, 1, 1), (0, 0), 'rl', 'eligibility', 'uperf', 0))
#g_ep_tuples.append(('../puddle-world/puddle-world-overgeneral.soar', (0, 0, 1, 1), (0, 0), 'rl', 'eligibility', 'uperf', 0.25))


parser = argparse.ArgumentParser(description='Run PuddleWorld experiments.')
parser.add_argument('-j', '--jobs', metavar='N', type=int,
                   action='store',
                   help='number of experiments to run in parallel')
parser.add_argument('-r', '--runs', metavar='N', type=int,
                   action='store', default=1,
                   help='number of runs per experiment')
parser.add_argument('-e', '--episodes', metavar='N', type=int,
                   action='store', default=1600,
                   help='number of episodes per run')

args = parser.parse_args()

if args.jobs is None:
  args.jobs = 'autodetect'


if not os.path.isdir(g_dir):
  os.mkdir(g_dir)
seeds = []
seeds_file = g_dir + '/seeds'
if os.path.isfile(seeds_file):
  f = open(seeds_file, 'r')
  for seed in f:
    seeds.append(int(seed))
  f.close()
if len(seeds) != args.runs:
  seeds = []
  for i in range(0, args.runs):
    seeds.append(random.randint(0,65535))
  f = open(seeds_file, 'w')
  for seed in seeds:
    f.write(str(seed) + '\n')
  f.close()
print str(seeds) + '\n'


class Experiment:
  def __init__(self, episodes, seed, rules, rl_rules_out, stdout, stderr, ep_tuple):
    self.episodes = episodes
    self.seed = seed
    self.rules = rules
    self.rl_rules_out = rl_rules_out
    self.stderr = stderr
    self.stdout = stdout
    
    self.ep_tuple = ep_tuple
    self.init_min_x = ep_tuple[1][0]
    self.init_min_y = ep_tuple[1][1]
    self.init_max_x = ep_tuple[1][2]
    self.init_max_y = ep_tuple[1][3]
    self.div_x = ep_tuple[2][0]
    self.div_y = ep_tuple[2][1]
    self.credit_assignment = ep_tuple[3]
    self.trace = ep_tuple[4]
    self.alpha = 'normal'
    self.variance = 'simple'
    self.refine = ep_tuple[5]
    self.refine_stddev = ep_tuple[6]
    self.sp = []
    for sp in ep_tuple[7:]:
      if len(sp) != 3:
        raise Exception("len(sp) != 3")
      self.sp.append(sp)
  
  def get_args(self):
    args = ['out/PuddleWorld',
            '--port', str(self.seed),
            '--episodes', str(self.episodes),
            '--seed', str(self.seed),
            '--rules', str(self.rules),
            '--rl-rules-out', str(self.rl_rules_out),
            '--initial', str(self.init_min_x), str(self.init_min_y), str(self.init_max_x), str(self.init_max_y),
            '--credit-assignment', str(self.credit_assignment),
            '--alpha', str(self.alpha),
            '--variance', str(self.variance),
            '--refine', str(self.refine),
            '--refine-stddev', str(self.refine_stddev)]
    if self.trace == 'tsdt':
      args += ['--tsdt']
    for sp in self.sp:
      args += ['--sp-special', str(sp[0]), str(sp[1]), str(sp[2])]
    return args
  
  def print_args(self):
    args = self.get_args()
    cmd = ''
    for arg in args:
      cmd += arg + ' '
    cmd += '> ' + self.stdout
    print cmd
  
  def run(self):
    args = self.get_args()
    f1 = open(self.stdout, 'w')
    f2 = open(self.stderr, 'w')
    subprocess.call(args, stderr=f2, stdout=f1)
    f2.close()
    f1.close()
    return self


dirs = []
experiments = []
for ep_tuple in g_ep_tuples:
  dir = ep_tuple[0].split('/')
  if len(dir) > 1:
    dir = dir[len(dir) - 1]
  else:
    dir = ep_tuple[0]
  dir = g_dir + '/' + dir + '_' + str(ep_tuple[2][0]) + '-' + str(ep_tuple[2][1])
  dir += '_' + str(ep_tuple[3])
  #dir += '_' + str(ep_tuple[4])
  dir += '_' + str(ep_tuple[5])
  dir += '_' + str(ep_tuple[6])
  for i in range(7, len(ep_tuple)):
    if len(ep_tuple[i]) != 3:
      raise Exception("ep_tuple[i] != 3")
    dir += '_' + str(ep_tuple[i][0]) + '-' + str(ep_tuple[i][1]) + '-' + str(ep_tuple[i][2])
  if not os.path.isdir(dir):
    os.mkdir(dir)
  dirs.append(dir)
  
  if ep_tuple[0] == g_rules:
    rules = dir + '/in.soar'
    shutil.copy(g_rules, rules)
    f = open(rules, 'a')
    f.write('sp {apply*initialize*puddleworld\n' +
            '    (state <s> ^operator.name puddleworld)\n' +
            '-->\n' +
            '    (<s> ^name puddleworld\n' +
            '        ^div <d>)\n' +
            '    (<d> ^name default\n' +
            '        ^x (/ 1.001 ' + str(ep_tuple[2][0]) + ')\n' +
            '        ^y (/ 1.001 ' + str(ep_tuple[2][1]) + '))\n' +
            '}\n');
    f.close()
  else:
    rules = ep_tuple[0]
    for sp in ep_tuple[7:]:
      print 'Not allowed to --sp-special arbitrary rules.'
      exit(1)
  
  for seed in seeds:
    rl_rules_out = dir + '/out-' + str(seed) + '.soar'
    output = dir + '/puddleworld-' + str(seed) + '.out'
    rl_rules_extra = dir + '/puddleworld-' + str(seed) + '.rl'
    experiment = Experiment(args.episodes, seed, rules, rl_rules_out, output, rl_rules_extra, ep_tuple)
    experiments.append(experiment)
    a = experiment.get_args()
    s = a[0]
    for ss in a[1:]:
      s += ' ' + ss
    print s

class Progress:
  def __init__(self, experiments):
    self.lock = thread.allocate_lock()
    
    self.count = {}
    self.finished = {}
    for experiment in experiments:
      try:
        self.count[experiment.ep_tuple] += 1
      except KeyError:
        self.count[experiment.ep_tuple] = 1
      self.finished[experiment.ep_tuple] = 0

  def just_finished(self, experiment):
    self.lock.acquire()
    self.finished[experiment.ep_tuple] += 1
    self.lock.release()

  def all_finished(self, ep_tuple):
    self.lock.acquire()
    num = self.count[ep_tuple]
    fin = self.finished[ep_tuple]
    self.lock.release()
    return fin is num

job_server = pp.Server(args.jobs)
progress = Progress(experiments)
start_time = time.time()
jobs = [(job_server.submit(Experiment.run, (experiment,), (), ('subprocess', 'thread',), callback=progress.just_finished, group=experiment.ep_tuple)) for experiment in experiments]

for ep_tuple, dir in zip(g_ep_tuples, dirs):
  while True:
    job_server.print_stats()
    if progress.all_finished(ep_tuple):
      break
    else:
      time.sleep(5)
  job_server.wait(ep_tuple)
  args = [g_plotter] + glob.glob(dir + '/*.out')
  print 'Plotting data for ' + str(ep_tuple) + '\n'
  subprocess.call(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)

print 'Total time elapsed: ', time.time() - start_time, 'seconds'
