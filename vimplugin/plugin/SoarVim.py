import vim
import sys, thread, time, re
import Python_sml_ClientInterface as sml

# log printing to buffer the connect or create command was called from?
_log_default = False

# kind of buggy right now
_scroll_lock = False

# a map from buffer numbers to the command to watch in that buffer
_watches = {}

# map of buffer numbers that get output logs to an array of levels they
# log
_log_bufs = {}

# the buffer number allocated to the preview buffer
_preview_buf = -1

# when scroll lock is on, this was the last time value at which
# a window was updated. Limit window updates to once every so many seconds
_last_win_update = 0
_min_time_between_updates = 1  # in seconds

# regular expressions to catch different levels of log output

# group 1 captures op id, group 2 captures op name
_w1op_regex = re.compile('^\\s*\\d+:\\s*O: ([A-Z]\\d+) \(([-_\\*\\|\\w]+)\)\\s*$')
# group 1 captures state id, group 2 captures reason for subgoaling
_w1sg_regex = re.compile('^\\s*\\d+:\\s*==>S: (S\\d+) \(([-_\\*\\|\\w ]+)\)\\s*$')

# group 1 captures phase name
#w2begin_regex = re.compile('^--- (\\w+) Phase ---$')
# group 1 captures phase name
#w2end_regex = re.compile('^--- END (\\w+) Phase ---$')

_w2_regex = re.compile('^\\s*---.*---\\s*$')

# group 1 captures firing or retracting, group 2 captures rule name
_w3_regex = re.compile('^(Firing|Retracting) ([-_\\*\\|\\w]+)$')

# group 1 captures in or out, group 2 captures the wme
_w4_regex = re.compile('(=>|<=)WM: (.*)$')

_w5_regex = re.compile('^\\s*\\([A-Z]\\d+ \\^[-_\\*\\w]+ [-_\\*\\|\\w]+ .*\\)\\s*$')

_w5arrow_regex = re.compile('^\\s*-->\\s*$')

_gds_add_regex = re.compile('^Adding to GDS for S[0-9]+')
_gds_del_regex = re.compile('^Removing state S[0-9]+ because element in GDS changed')

_all_regex = {
		_w1op_regex    : 1,
		_w1sg_regex    : 1, 
		_w2_regex      : 2,
		_w3_regex      : 3,
		_w4_regex      : 4,
		_w5_regex      : 5,
		_w5arrow_regex : 5,
	  _gds_add_regex : 6,
		_gds_del_regex : 6 }

# list of list of tuples, d1 = step number, d2 = list of output, tuple = (watch level, output)
_watch_log = [[]]

# the current step
_curr_step = 0

_kernel = None
_agent = None

def vim_get_bufnum(buf):
	for i in range(len(vim.buffers)):
		if vim.buffers[i] == buf:
			return i
	raise Exception, "Buffer not found"

def get_curr_bufn():
	return int(vim.eval('bufnr("%")')) - 1

def get_curr_winn():
	return int(vim.eval('bufwinnr("%")')) - 1

def get_winn(bufn):
	return int(vim.eval('bufwinnr(%s)' % (bufn+1))) - 1

def set_statusline(s):
	vim.command("setlocal statusline=%s" % s.replace(' ', '\\ '))

def clear_statusline():
	vim.command("setlocal statusline=")

def vim_multiline_to_buf(buf, s):
	for line in s.split('\n'):
		if len(line.strip()) > 0:
			buf.append(line)

# this is the preview buffer for soar, not tags
def get_preview_buf():
	global _preview_buf

	window_closed = True

	if _preview_buf >= 0:
		for w in vim.windows:
			if w.buffer == vim.buffers[_preview_buf]:
				window_closed = False
				break

	if _preview_buf < 0 or window_closed:
		bn = get_curr_bufn()
		vim.command('5new +set\\ bt=nofile')
		set_statusline("preview window")
		_preview_buf = get_curr_bufn()

		# jump focus back to current window
		wn = get_winn(bn)
		vim.command('exec "%dwincmd w"' % (wn + 1))
	
	return _preview_buf

def exec_cmd_buf(cmd):
	global _kernel
	global _agent
	
#		src = SoarCmd.get_source(cmd, \
#					"_kernel", \
#					'vim.current.buffer.append(__output.split(\'\\n\'))', \
#					_agent.GetAgentName())
	output = _kernel.ExecuteCommandLine(cmd, _agent.GetAgentName())
	vim_multiline_to_buf(vim.current.buffer, output)

	# don't jump cursor to new position, can easily do it with G
	#vim.current.window.cursor = (len(vim.current.buffer), 0)

def exec_cmd_prev(cmd):
	global _kernel
	global _agent
	
	output = _kernel.ExecuteCommandLine(cmd, _agent.GetAgentName())
	pbuf = vim.buffers[get_preview_buf()]
	pbuf[:] = None
	vim_multiline_to_buf(pbuf, output)
	
def exec_cmd(cmd):
	global _kernel
	global _agent
	
	output = _kernel.ExecuteCommandLine(cmd, _agent.GetAgentName())
	print output

#def exec_cmd_thread(cmd):
#	"""Runs soar command in a separate thread, for blocking calls like 'run'"""
#	global _kernel
#	global _agent
#	
#	_kernel.ExecuteCommandLine(cmd, _agent.GetAgentName())
#	print "RETURNED"

def create_kernel(agent_name = 'soar1'):
	global _kernel
	global _agent
	global _log_default
	
	_kernel = sml.Kernel.CreateKernelInNewThread()
	if _kernel == None:
		raise Exception, "Soar _kernel creation failed"
	
	_agent = _kernel.CreateAgent(agent_name)
	if _agent == None:
		raise Exception, "_agent creation failed: %s" % _kernel.GetLastErrorDescription()
	
	if _log_default:
		log_print()
	
	_agent.RegisterForPrintEvent(sml.smlEVENT_PRINT, _print_callback, None)
	_agent.RegisterForRunEvent(sml.smlEVENT_AFTER_RUN_ENDS, _watch_callback, None)
	print "Kernel created successfully"

def connect(host='127.0.0.1', port=12121, agent_num=0):
	global _kernel
	global _agent
	global _log_default
	
	_kernel = sml.Kernel.CreateRemoteConnection(True, host, port)
	if _kernel == None:
		raise Exception, "Could not connect to remote _kernel"
	
	_agent = _kernel.GetAgentByIndex(0)
	if _agent == None:
		raise Exception, "Could not find _agent on remote _kernel"
	
	if _log_default:
		log_print()
	
	_agent.RegisterForPrintEvent(sml.smlEVENT_PRINT, _print_callback, None)
	_agent.RegisterForRunEvent(sml.smlEVENT_AFTER_RUN_ENDS, _watch_callback, None)
	print "Connected to kernel succesfully"
	
def _print_callback(id, userData, agent, message):
	global _scroll_lock
	global _log_bufs
	global _last_win_update
	global _watch_log
	global _curr_step

	for line in message.split('\n'):
		line_matched = False
		for regex, level in _all_regex.items():
			if regex.match(line) != None:
				line_matched = True
				if level == 1:
					_curr_step += 1
					_watch_log.append([])

				_watch_log[_curr_step].append((level, line))

				# print the line to the buffers that should get it
				for lbufn in _log_bufs:
					if level in _log_bufs[lbufn]:
						vim.buffers[lbufn].append(line)

		if not line_matched and len(line.strip()) > 0:
			# assume this is user output
			_watch_log[_curr_step].append((0, line))
			for lbufn in _log_bufs:
				if 0 in _log_bufs[lbufn]:
					vim.buffers[lbufn].append(line)

	for bufn in _log_bufs:
		vim.windows[get_winn(bufn)].cursor = (len(vim.buffers[bufn]), 0)

	redraw = False
	t = time.clock()
	if t - _last_win_update > _min_time_between_updates:
		_last_win_update = t
		redraw = True

	if _scroll_lock and redraw:
		vim.command('windo redraw')
	
def log_print(levels=[0,1]):
	global _log_bufs

	bufn = get_curr_bufn()
	if bufn in _watches:
		print "Can't log to a watch buffer"
	else:
		set_statusline("Logging levels %s" % str(levels))
		_log_bufs[bufn] = levels

def unlog_print():
	global _log_bufs

	clear_statusline()
	_log_bufs.pop(get_curr_bufn(), None)

def _watch_callback(id, userData, agent, phase):
	global _kernel
	global _watches

	for bufn, cmd in _watches.items():
		try:
			buf = vim.buffers[bufn]
		except IndexError:
			# this buffer no longer exists, remove it from the watch
			_watches.pop[bufn]
			continue
		
		buf[:] = None
		output = _kernel.ExecuteCommandLine(cmd, agent.GetAgentName())
		
		buf.append('')
		vim_multiline_to_buf(buf, output)

def watch(cmd):
	global _agent
	global _log_bufs
	global _watches
	
	bufn = get_curr_bufn()
	if bufn in _log_bufs:
		print "Can't watch in a log buffer"
	else:
		_watches[bufn] = cmd
		set_statusline("Watching %s" % cmd)
		_watch_callback(0, None, _agent, 0)

def unwatch():
	global _watches

	bufn = get_curr_bufn()
	_watches.pop(bufn, None)
	clear_statusline()

def print_log(steps = None, levels = None):
	global _watch_log

	if steps == None:
		steps = [len(_watch_log) - 1]
	elif steps == []:
		steps = range(_curr_step + 1)

	if levels == None:
		levels = range(_watch_log.values()[-1])

	for s in steps:
		if s in range(len(_watch_log)):
			for line in _watch_log[s]:
				if line[0] in levels:
					vim.current.buffer.append(line[1])

def init_agent():
	exec_cmd('init')
	_watch_log = [[]]
	_curr_step = 0
