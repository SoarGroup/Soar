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

	username = None
	cursor = None

	KFACTOR = 32

	def __init__(self, cursor, tank):
		self.cursor = cursor
		(self.id, self.userid, self.tankname, self.wins, self.losses, self.draws, self.unfinished, self.pointspermatch, self.sourcefile, self.active, self.fighting, self.ladderpoints, self.rating) = tank
		self.cursor.execute("SELECT username FROM users WHERE userid=%s", (self.userid,))
		(self.username,) = self.cursor.fetchone()

	def set_fighting(self, setting):
		if setting:
			self.fighting = 1
		else:
			self.fighting = 0
		self.cursor.execute("UPDATE tanks SET fighting=%s WHERE tankid=%s", (self.fighting, self.id,))
	
	def update_record(self, winner, draw, finished, score, opponentrating):
		credit = float(0)
		if winner:
			credit = float(1)
			self.wins += 1
			self.ladderpoints += 2
		elif draw:
			credit = 0.5
			self.draws += 1
			self.ladderpoints += 1
		else:
			self.losses +=1

		if finished:
			self.ladderpoints += 1
		
		matches = self.wins + self.losses + self.draws
		self.pointspermatch = ((self.pointspermatch*(matches-1)) + score) / matches
		
		expectedcredit = 1 / (1 + (float(10)**((opponentrating - self.rating)/float(400))))
		newrating = int(self.rating + self.KFACTOR * (credit - expectedcredit))
		logging.debug("%s: %d = %d + %d * (%f - %f)", self.get_full_name(), newrating, self.rating, self.KFACTOR, credit, expectedcredit)
		self.rating = newrating
		
		self.cursor.execute("UPDATE tanks SET pointspermatch=%s, ladderpoints=%s, wins=%s, losses=%s, draws=%s, rating=%s WHERE tankid=%s", (self.pointspermatch, self.ladderpoints, self.wins, self.losses, self.draws, self.rating, self.id))

	def increment_unfinished(self):
		self.unfinished += 1
		self.cursor.execute("UPDATE tanks SET unfinished=%s WHERE tankid=%s", (self.unfinished, self.id))

	def get_info_string(self):
		return "%s: %d points (%d-%d-%d)" % (self.get_full_name(), self.ladderpoints, self.wins, self.losses, self.draws,)
	
	def get_full_name(self):
		return "%s.%s" % (self.username, self.tankname,)
		
	def get_full_source(self):
		return "tanks/%s/%s/%s" % (self.username, self.tankname, self.sourcefile,)
	
					
