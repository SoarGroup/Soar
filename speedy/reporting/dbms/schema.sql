
CREATE TABLE IF NOT EXISTS experiments (
  exp_id int(11) NOT NULL AUTO_INCREMENT,
  exp_name varchar(250) NOT NULL,
  PRIMARY KEY (exp_id),
  UNIQUE KEY exp_name (exp_name)
);

CREATE TABLE IF NOT EXISTS exp_schemas (
  exp_id int(11) NOT NULL,
  field_id int(11) NOT NULL,
  field_name varchar(250) NOT NULL,
  field_type int(11) NOT NULL,
  KEY exp_id (exp_id,field_id)
);
