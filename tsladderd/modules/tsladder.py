import MySQLdb
import logging

class Tank:
	"A class representing a tank structure in MySQL"
	
	id = None
	userid = None
	tankname = None
	wins = None
	losses = None
	draws = None
	unfinished = None
	pointspermatch = None
	sourcefile = None
	active = None
	fighting = None
	ladderpoints = None
	rating = None
	snclosses = None
	avgtime = None

	username = None
	cursor = None

	KFACTOR = 32

	last_score = None
	last_finished = None
	last_status = None
	last_opponent_rating = None
	last_elapsed = None

	def __init__(self, cursor, tank):
		self.cursor = cursor
		(self.id, self.userid, self.tankname, self.wins, self.losses, self.draws, self.unfinished, self.pointspermatch, self.sourcefile, self.active, self.fighting, self.ladderpoints, self.rating, self.snclosses, self.avgtime) = tank
		self.cursor.execute("SELECT username FROM users WHERE userid=%s", (self.userid,))
		(self.username,) = self.cursor.fetchone()

	def set_fighting(self, setting):
		if setting:
			self.fighting = 1
		else:
			self.fighting = 0
		self.cursor.execute("UPDATE tanks SET fighting=%s WHERE tankid=%s", (self.fighting, self.id,))
	
	def set_last_match(self, score, finished, status, opponent_rating, elapsed):
		self.last_score = score 
		self.last_finished = finished
		self.last_status = status
		self.last_opponent_rating = opponent_rating
		self.last_elapsed = elapsed
	
	def increment_interrupted(self, elapsed):
		self.snclosses += 1
		self.cursor.execute("UPDATE tanks SET snclosses=%s WHERE tankid=%s", (self.snclosses, self.id))

		credit = float(0)
		expectedcredit = 1 / (1 + (float(10)**((1000 - self.rating)/float(400))))
		logging.debug("%s: expectedcredit: %f = 1 / (1 + (10**((1000 - %d)/400)))", self.get_full_name(), expectedcredit, self.rating)
		newrating = int(self.rating + self.KFACTOR * (credit - expectedcredit))
		logging.debug("%s: newrating: %d = %d + %d * (%f - %f)", self.get_full_name(), newrating, self.rating, self.KFACTOR, credit, expectedcredit)
		self.rating = newrating
		self.losses += 1
		
		matches = self.wins + self.losses + self.draws
		self.pointspermatch = ((self.pointspermatch*(matches-1)) + 0) / matches
		self.avgtime = ((self.avgtime*(matches-1)) + elapsed) / matches
		
		self.cursor.execute("UPDATE tanks SET pointspermatch=%s, avgtime=%s, losses=%s, rating=%s WHERE tankid=%s", (self.pointspermatch, self.avgtime, self.losses, self.rating, self.id))
		
	def update_record(self):
		credit = float(0)
		if self.last_status == "winner":
			credit = float(1)
			self.wins += 1
			self.ladderpoints += 2
			if self.last_finished:
				self.ladderpoints += 1
			else:
				self.unfinished += 1
		elif self.last_status == "draw":
			credit = 0.5
			self.draws += 1
			self.ladderpoints += 1
		else:
			self.losses +=1


		matches = self.wins + self.losses + self.draws
		self.pointspermatch = ((self.pointspermatch*(matches-1)) + self.last_score) / matches
		self.avgtime = ((self.avgtime*(matches-1)) + self.last_elapsed) / matches
		
		expectedcredit = 1 / (1 + (float(10)**((self.last_opponent_rating - self.rating)/float(400))))
		logging.debug("%s: expectedcredit: %f = 1 / (1 + (10**((%d - %d)/400)))", self.get_full_name(), expectedcredit, self.last_opponent_rating, self.rating)
		newrating = int(self.rating + self.KFACTOR * (credit - expectedcredit))
		logging.debug("%s: newrating: %d = %d + %d * (%f - %f)", self.get_full_name(), newrating, self.rating, self.KFACTOR, credit, expectedcredit)
		self.rating = newrating
		
		self.cursor.execute("UPDATE tanks SET pointspermatch=%s, ladderpoints=%s, wins=%s, losses=%s, draws=%s, rating=%s, unfinished=%s, avgtime=%s WHERE tankid=%s", (self.pointspermatch, self.ladderpoints, self.wins, self.losses, self.draws, self.rating, self.unfinished, self.avgtime, self.id))

	def get_info_string(self):
		return "%s: %d points (%d-%d-%d)" % (self.get_full_name(), self.ladderpoints, self.wins, self.losses, self.draws,)
	
	def get_full_name(self):
		return "%s.%s" % (self.username, self.tankname,)
		
	def get_full_source(self):
		return "tanks/%s/%s/%s" % (self.username, self.tankname, self.sourcefile,)
	
					
