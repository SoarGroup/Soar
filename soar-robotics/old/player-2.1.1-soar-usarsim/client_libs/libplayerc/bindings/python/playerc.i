
%module playerc

%{
#include "playerc.h"
%}

%include "typemaps.i"

// Provide array access to the RFID tags
// We will return a list of tuples. Each tuple contains the RFID type, and the RFID ID.
//  The RFID ID is a Python string containing the unprocessed bytes of the RFID tag.
//  In Python, use   rfid.tags[i][1].encode('hex')   to see the regular Hex-ASCII representation.
%typemap(out) playerc_rfidtag_t [ANY]
{
  int i;
  $result = PyList_New($1_dim0);  //we will return a Python List

  //At this level, we don't get the ammount of tags, so we just produce a long list, of size PLAYER_RFID_MAX_TAGS
  //(this is the size of the array of playerc_rfidtag_t contained in the RFID data packet)
  for (i = 0; i != $1_dim0; ++i)
  {

    PyObject *tuple = PyTuple_New(2); // we will make a tuple with 2 fields: type(int) and RFID ID(string)

    const unsigned int blength=PLAYERC_RFID_MAX_GUID;
    char buffer[blength];
    memset(buffer, 0, blength );

    //result is of type playerc_rfidtag_t
    unsigned int j;    
    unsigned int guid_count=$1[i].guid_count;

    //copy the bytes into the buffer
    for (j=0 ; j != guid_count ; ++j) {
	buffer[j]=$1[i].guid[j];
    }

    //generate a Python string from the buffer
    PyObject *ostring = PyString_FromStringAndSize(buffer,guid_count);

    //generate an Int from the tag type
    PyObject *otype = PyInt_FromLong($1[i].type);

    //set the previous objects into the tuple
    PyTuple_SetItem(tuple,0,otype);
    PyTuple_SetItem(tuple,1,ostring);

    //set the tupple into the corresponding place of the list
    PyList_SetItem($result,i,tuple);
  }

  //$result is the Python List that gets returned automatically at the end of this function
}


// For playerc_simulation_get_pose2d()
%apply double *OUTPUT { double *x, double *y, double *a };

// Provide array (write) access
%typemap(in) double [ANY] (double temp[$1_dim0])
{
  int i;
  if (!PySequence_Check($input))
  {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  if (PySequence_Length($input) != $1_dim0) {
    PyErr_SetString(PyExc_ValueError,"Size mismatch. Expected $1_dim0 elements");
    return NULL;
  }
  for (i = 0; i < $1_dim0; i++)
  {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyNumber_Check(o))
    {
      temp[i] = (float) PyFloat_AsDouble(o);    }
    else
    {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");
      return NULL;
    }
  }
  $1 = temp;
}

%typemap(in) float [ANY] (float temp[$1_dim0])
{
  int i;
  if (!PySequence_Check($input))
  {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  if (PySequence_Length($input) != $1_dim0) {
    PyErr_SetString(PyExc_ValueError,"Size mismatch. Expected $1_dim0 elements");
    return NULL;
  }
  for (i = 0; i < $1_dim0; i++)
  {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyNumber_Check(o))
    {
      temp[i] = (float) PyFloat_AsDouble(o);    }
    else
    {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");
      return NULL;
    }
  }
  $1 = temp;
}

%typemap(in) uint8_t data[]
{
	int temp = 0;
	// Check if is a list
			if (PyList_Check ($input))
	{
		int size = PyList_Size ($input);
		int ii = 0;
		$1 = (uint8_t*) malloc (size * sizeof (uint8_t));
		for (ii = 0; ii < size; ii++)
		{
			PyObject *o = PyList_GetItem ($input, ii);
			temp = PyInt_AsLong (o);
			if (temp == -1 && PyErr_Occurred ())
			{
				free ($1);
				return NULL;
			}
			$1[ii] = (uint8_t) temp;
		}
	}
	else
	{
		PyErr_SetString (PyExc_TypeError, "not a list");
		return NULL;
	}
}

/*// typemap to free the array created in the previous typemap
%typemap(freearg) uint8_t data[]
{
	if ($input) free ((uint8_t*) $input);
}*/

// typemap for passing points into the graphics2d interface
%typemap(in) player_point_2d_t pts[]
{
	// Check if is a list
	if (PyList_Check ($input))
	{
		int size = PyList_Size ($input);
		int ii = 0;
		$1 = (player_point_2d_t*) malloc (size * sizeof (player_point_2d_t));
		for (ii = 0; ii < size; ii++)
		{
			PyObject *o = PyList_GetItem ($input, ii);
			if (PyTuple_Check (o))
			{
				if (PyTuple_GET_SIZE (o) != 2)
				{
					PyErr_SetString (PyExc_ValueError, "tuples must have 2 items");
					free ($1);
					return NULL;
				}
				$1[ii].px = PyFloat_AsDouble (PyTuple_GET_ITEM (o, 0));
				$1[ii].py = PyFloat_AsDouble (PyTuple_GET_ITEM (o, 1));
			}
			else
			{
				PyErr_SetString (PyExc_TypeError, "list must contain tuples");
				free ($1);
				return NULL;
			}
		}
	}
	else
	{
		PyErr_SetString (PyExc_TypeError, "not a list");
		return NULL;
	}
}

// typemap for passing 3d points into the graphics3d interface
%typemap(in) player_point_3d_t pts[]
{
	// Check if is a list
	if (PyList_Check ($input))
	{
		int size = PyList_Size ($input);
		int ii = 0;
		$1 = (player_point_3d_t*) malloc (size * sizeof (player_point_3d_t));
		for (ii = 0; ii < size; ii++)
		{
			PyObject *o = PyList_GetItem ($input, ii);
			if (PyTuple_Check (o))
			{
				if (PyTuple_GET_SIZE (o) != 3)
				{
					PyErr_SetString (PyExc_ValueError, "tuples must have 3 items");
					free ($1);
					return NULL;
				}
				$1[ii].px = PyFloat_AsDouble (PyTuple_GET_ITEM (o, 0));
				$1[ii].py = PyFloat_AsDouble (PyTuple_GET_ITEM (o, 1));
				$1[ii].pz = PyFloat_AsDouble (PyTuple_GET_ITEM (o, 2));
			}
			else
			{
				PyErr_SetString (PyExc_TypeError, "list must contain tuples");
				free ($1);
				return NULL;
			}
		}
	}
	else
	{
		PyErr_SetString (PyExc_TypeError, "not a list");
		return NULL;
	}
}

// typemap to free the array created in the previous typemap
%typemap(freearg) player_point2d_t pts[]
{
	if ($input) free ((player_point2d_t*) $input);
}

// typemap for tuples to colours
%typemap(in) player_color_t (player_color_t temp)
{
	// Check it is a tuple
	if (PyTuple_Check ($input))
	{
		// Check the tuple has four elements
		if (PyTuple_GET_SIZE ($input) != 4)
		{
			PyErr_SetString (PyExc_ValueError, "tuple must have 4 items");
			return NULL;
		}
		temp.alpha = PyInt_AsLong (PyTuple_GET_ITEM ($input, 0));
		temp.red = PyInt_AsLong (PyTuple_GET_ITEM ($input, 1));
		temp.green = PyInt_AsLong (PyTuple_GET_ITEM ($input, 2));
		temp.blue = PyInt_AsLong (PyTuple_GET_ITEM ($input, 3));
	}
	else
	{
		PyErr_SetString (PyExc_TypeError, "not a tuple");
		return NULL;
	}
}

// Provide array (write) access
%typemap(in) double [ANY][ANY] (double temp[$1_dim0][$1_dim1])
{
  int i,j;
  if (!PySequence_Check($input))
  {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  if (PySequence_Length($input) != $1_dim0) {
    PyErr_SetString(PyExc_ValueError,"Size mismatch. Expected $1_dim0 elements");
    return NULL;
  }
  for (i = 0; i < $1_dim0; i++)
  {
    PyObject *olist = PySequence_GetItem($input,i);
    if (!PySequence_Check(olist))
    {
      PyErr_SetString(PyExc_ValueError,"Expected a sequence");
      return NULL;
    }
    if (PySequence_Length(olist) != $1_dim1) {
      PyErr_SetString(PyExc_ValueError,"Size mismatch. Expected $1_dim1 elements");
      return NULL;
    }

    for (j = 0; j < $1_dim1; j++)
    {
      PyObject *o = PySequence_GetItem(olist,j);

      if (PyNumber_Check(o))
      {
        temp[i][j] = (float) PyFloat_AsDouble(o);
      }
      else
      {
        PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");
        return NULL;
      }
    }
  }
  $1 = temp;
}

%typemap(in) uint8_t
{
  $1 = (uint8_t) PyLong_AsLong($input);
}

%typemap(in) uint32_t
{
  $1 = (uint32_t) PyLong_AsLong ($input);
}

// Integer types
%typemap(out) uint8_t
{
  $result = PyInt_FromLong((long) (unsigned char) $1);
}

%typemap(out) uint16_t
{
  $result = PyInt_FromLong((long) (unsigned long) $1);
}

%typemap(out) uint32_t
{
  $result = PyInt_FromLong((long) (unsigned long long) $1);
}

// Provide array access
%typemap(out) double [ANY]
{
  int i;
  $result = PyList_New($1_dim0);
  for (i = 0; i < $1_dim0; i++)
  {
    PyObject *o = PyFloat_FromDouble((double) $1[i]);
    PyList_SetItem($result,i,o);
  }
}

%typemap(out) float [ANY]
{
  int i;
  $result = PyList_New($1_dim0);
  for (i = 0; i < $1_dim0; i++)
  {
    PyObject *o = PyFloat_FromDouble((double) $1[i]);
    PyList_SetItem($result,i,o);
  }
}

%typemap(out) uint8_t [ANY]
{
  $result = PyBuffer_FromMemory((void*)$1,sizeof(uint8_t)*$1_dim0);
}

// Provide array access doubly-dimensioned arrays
%typemap(out) double [ANY][ANY]
{
  int i, j;
  $result = PyList_New($1_dim0);
  for (i = 0; i < $1_dim0; i++)
  {
    PyObject *l = PyList_New($1_dim1);
    for (j = 0; j < $1_dim1; j++)
    {
      PyObject *o = PyFloat_FromDouble((double) $1[i][j]);
      PyList_SetItem(l,j,o);
    }
    PyList_SetItem($result,i,l);
  }
}

// Catch-all rule to converts arrays of structures to tuples of proxies for
// the underlying structs
%typemap(out) SWIGTYPE [ANY]
{
 int i;
  $result = PyTuple_New($1_dim0);
  for (i = 0; i < $1_dim0; i++)
  {
    PyObject *o = SWIG_NewPointerObj($1 + i, $1_descriptor, 0);
    PyTuple_SetItem($result,i,o);
  }
}

// Provide thread-support on some functions
%exception read
{
  Py_BEGIN_ALLOW_THREADS
  $action
  Py_END_ALLOW_THREADS
}


// Include Player header so we can pick up some constants and generate
// wrapper code for structs
%include "../../../../libplayercore/player.h"
%include "../../../../libplayercore/player_interfaces.h"


// Use this for regular c-bindings;
// e.g. playerc_client_connect(client, ...)
//%include "../../playerc.h"


// Use this for object-oriented bindings;
// e.g., client.connect(...)
// This file is created by running ../parse_header.py
%include "playerc_oo.i"

%extend playerc_laser
{
double get_range (int index)
{
	return self->ranges[index];
};
}

%extend playerc_blackboard
{
static void python_on_blackboard_event(playerc_blackboard_t *device, player_blackboard_entry_t entry)
{
	char * str;
	int i;
	double d;
	PyObject *data, *entry_dict, *group_dict, *groups, *list;
	groups = (PyObject *)PyTuple_GetItem(device->py_private,0);
	assert(groups);

	group_dict = (PyObject*) PyDict_GetItemString(groups, entry.group);
	if (!group_dict)
	{
		return;
	}
	
	// find our key in the group, if it exists
	entry_dict = (PyObject*) PyDict_GetItemString(group_dict, entry.key);
	if (!entry_dict)
	{
		return;
	}

	PyDict_SetItemString(entry_dict, "type", PyLong_FromLong(entry.type));
	PyDict_SetItemString(entry_dict, "subtype", PyLong_FromLong(entry.subtype));
	PyDict_SetItemString(entry_dict, "timestamp_sec", PyLong_FromLong(entry.timestamp_sec));
	PyDict_SetItemString(entry_dict, "timestamp_usec", PyLong_FromLong(entry.timestamp_usec));
	list = (PyObject*)PyTuple_GetItem(device->py_private,1);
	assert(list);
	data = NULL;
    switch(entry.subtype)
    {
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_NONE:
    		data = Py_None;
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_STRING:
    		assert(entry.type == PLAYERC_BLACKBOARD_DATA_TYPE_COMPLEX);
		  	str = malloc(entry.data_count);
  			assert(str);
  			memcpy(str, entry.data, entry.data_count);
    		data = PyString_FromString(str);
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_INT:
    		assert(entry.type == PLAYERC_BLACKBOARD_DATA_TYPE_SIMPLE);
		 	i = 0;
  			memcpy(&i, entry.data, entry.data_count);
    		data = PyLong_FromLong(i);
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_DOUBLE:
    		assert(entry.type == PLAYERC_BLACKBOARD_DATA_TYPE_SIMPLE);
  			d = 0.0;
  			memcpy(&d, entry.data, entry.data_count);
    		data = PyFloat_FromDouble(d);
    	break;
    	default:
    		data = Py_None;
    	break;
    }

	assert(data != NULL);
	PyDict_SetItemString(entry_dict, "data", data);
	
	if (PyLong_AsLong(PyTuple_GetItem(device->py_private,2)) > 0)
	{
		list = (PyObject*)PyTuple_GetItem(device->py_private,1);
		assert(list);
		PyList_Append(list, PyDict_Copy(entry_dict));
	}
}

// Will always return a list object. Returns a list of dictionary objects containing the event data.
PyObject *get_events()
{
  PyObject *list, *copy;
  int i, j;
  assert(self->py_private);

  list = (PyObject*)PyTuple_GetItem(self->py_private,1);
  copy = PyList_New(0);
  j = PyList_Size(list);
  for (i = 0; i < j; i++)
  {
    PyList_Append(copy, PyList_GetItem(list, 0));
    PySequence_DelItem(list,0);
  }
  return copy;
}

void set_queue_events(PyObject *boolean)
{
  if(!PyBool_Check(boolean))
  {
    PyErr_SetString(PyExc_RuntimeError, "Expected a bool object.");
    return;
  }
  if (boolean==Py_False)
  {
    PyTuple_SetItem(self->py_private,2,PyLong_FromLong(0));
  }
  else
  {
    PyTuple_SetItem(self->py_private,2,PyLong_FromLong(1));
  }
}

void unsubscribe_from_key_py(const char *key, const char *group)
{
	long refcount;
	PyObject *group_dict, *entry_dict, *groups;

	groups = (PyObject *)PyTuple_GetItem(self->py_private,0);
	assert(groups);

	group_dict = (PyObject*) PyDict_GetItemString(groups, group);
	if (!group_dict)
	{
		PyErr_SetString(PyExc_RuntimeError, "Group does not exist");
		return;
	}
	
	// find our key in the group, if it exists
	entry_dict = (PyObject*) PyDict_GetItemString(group_dict, key);
	if (!entry_dict)
	{
		PyErr_SetString(PyExc_RuntimeError, "Key does not exist");
		return;
	}
	refcount = PyLong_AsLong(PyDict_GetItemString(entry_dict, "___refcount"));
	if(refcount==0)
	{
		PyErr_SetString(PyExc_RuntimeError, "No subscription to key to unsubscribe from.");
		return;
	}
	if(refcount==1)
	{
		playerc_blackboard_unsubscribe_from_key(self, key, group);
	}
	PyDict_SetItemString(entry_dict, "___refcount", PyLong_FromLong(refcount-1));
}

PyObject * subscribe_to_key_py(const char *key, const char *group)
{
	char *str;
	int i, result, refcount;
	double d;
	PyObject *data, *entry_dict, *group_dict, *groups;
	player_blackboard_entry_t *entry;

	groups = (PyObject *)PyTuple_GetItem(self->py_private,0);
	assert(groups);
	// grab our group dict, if it doesnt exist create a new one
	group_dict = (PyObject*) PyDict_GetItemString(groups, group);
	if (!group_dict)
	{
		group_dict = PyDict_New();
		assert (group_dict);
		result = PyDict_SetItemString(groups,group,group_dict);
		if (result != 0)
		{
			PyErr_SetString(PyExc_RuntimeError, "Failed to set group key");
			return Py_None;
		}
	}
	// find our key in the group, if it exists, create it if not
	entry_dict = (PyObject*) PyDict_GetItemString(group_dict, key);
	if (!entry_dict)
	{
		entry_dict = PyDict_New();
		assert (entry_dict);
		PyDict_SetItemString(entry_dict, "key", PyString_FromString(key));
		PyDict_SetItemString(entry_dict, "group", PyString_FromString(group));
		PyDict_SetItemString(entry_dict, "___refcount", PyLong_FromLong(1));
	}
	else
	{
		// key already exists, so increment our ref count and return current value
		refcount = PyLong_AsLong(PyDict_GetItemString(entry_dict, "___refcount"));
		if(refcount>=1000)
		{
			PyErr_SetString(PyExc_RuntimeError, "Reached over 1000 subscriptions");
		}
		if(refcount!=0)
		{
			PyDict_SetItemString(entry_dict, "___refcount", PyLong_FromLong(refcount+1));
			Py_INCREF(entry_dict);
			return entry_dict;
		}
	}
	assert(self);
	result = playerc_blackboard_subscribe_to_key(self, key, group, &entry);
	if (result != 0)
	{
		PyErr_SetString(PyExc_RuntimeError, "Failed to subscribe to key");
		return Py_None;
	}
	assert(entry);
	assert(entry->key);
	assert(entry->key_count > 0);
	PyDict_SetItemString(entry_dict, "type", PyLong_FromLong(entry->type));
	PyDict_SetItemString(entry_dict, "subtype", PyLong_FromLong(entry->subtype));
	PyDict_SetItemString(entry_dict, "timestamp_sec", PyLong_FromLong(entry->timestamp_sec));
	PyDict_SetItemString(entry_dict, "timestamp_usec", PyLong_FromLong(entry->timestamp_usec));
    switch(entry->subtype)
    {
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_NONE:
    		data = Py_None;
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_STRING:
    		assert(entry->type == PLAYERC_BLACKBOARD_DATA_TYPE_COMPLEX);
		  	str = malloc(entry->data_count);
  			assert(str);
  			memcpy(str, entry->data, entry->data_count);
    		data = PyString_FromString(str);
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_INT:
    		assert(entry->type == PLAYERC_BLACKBOARD_DATA_TYPE_SIMPLE);
		 	i = 0;
  			memcpy(&i, entry->data, entry->data_count);
    		data = PyLong_FromLong(i);
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_DOUBLE:
    		assert(entry->type == PLAYERC_BLACKBOARD_DATA_TYPE_SIMPLE);
  			d = 0.0;
  			memcpy(&d, entry->data, entry->data_count);
    		data = PyFloat_FromDouble(d);
    	break;
    	default:
    		data = Py_None;
    	break;
    	
    }
    PyDict_SetItemString(entry_dict, "data", data);
	result = PyDict_SetItemString(group_dict,key,entry_dict);
	if (result != 0)
	{
		PyErr_SetString(PyExc_RuntimeError, "Failed to create key dictionary entry");
		return Py_None;
	}
    free(entry->key);
    free(entry->group);
    free(entry->data);
    free(entry);
    Py_INCREF(entry_dict);
    return entry_dict;
  }

PyObject * subscribe_to_key(const char *key, const char *group)
{
	return playerc_blackboard_subscribe_to_key_py( self, key, group);
}

PyObject * get_value(const char *key, const char *group)
{
	PyObject *group_dict, *entry_dict, *groups;
	int refcount;
	
	groups = (PyObject *)PyTuple_GetItem(self->py_private,0);
	assert(groups);

	group_dict = (PyObject*) PyDict_GetItemString(groups, group);
	if (!group_dict)
		return playerc_blackboard_subscribe_to_key_py( self, key, group);
	// find our key in the group, if it exists
	entry_dict = (PyObject*) PyDict_GetItemString(group_dict, key);
	if (!entry_dict)
		return playerc_blackboard_subscribe_to_key_py( self, key, group);
	refcount = PyLong_AsLong(PyDict_GetItemString(entry_dict, "___refcount"));
	if(refcount==0)
		return playerc_blackboard_subscribe_to_key_py( self, key, group);
	Py_INCREF(entry_dict);
	return entry_dict;
}
  
  void set_entry_raw(player_blackboard_entry_t *entry)
  {
    playerc_blackboard_set_entry(self,entry);
  }
  
  PyObject *set_entry(PyObject *dict)
  {
	PyObject *key;
	PyObject *group;
	PyObject *type;
	PyObject *subtype;
	PyObject *timestamp_sec;
	PyObject *timestamp_usec;
	PyObject *data;
	char *str;
	int i, result;
	double d;
	int length;
	
	if (!PyDict_Check(dict))
	{
		PyErr_SetString(PyExc_RuntimeError, "Expected a dictionary object.");
		return PyLong_FromLong(-1);
	}

    player_blackboard_entry_t entry;
    memset(&entry, 0, sizeof(entry));
    
    key = PyDict_GetItem(dict, PyString_FromString("key"));
    group = PyDict_GetItem(dict, PyString_FromString("group"));
	type = PyDict_GetItem(dict, PyString_FromString("type"));
	subtype = PyDict_GetItem(dict, PyString_FromString("subtype"));
	timestamp_sec = PyDict_GetItem(dict, PyString_FromString("timestamp_sec"));
	timestamp_usec = PyDict_GetItem(dict, PyString_FromString("timestamp_usec"));
	data = PyDict_GetItem(dict, PyString_FromString("data"));
	
	if (key == NULL || group == NULL || type == NULL || subtype == NULL || timestamp_sec == NULL || timestamp_usec == NULL || data == NULL)
	{
		PyErr_SetString(PyExc_RuntimeError, "Dictionary object missing keys.");
		return PyLong_FromLong(-1);
	}
	
	if (!PyString_Check(key))
	{
		PyErr_SetString(PyExc_RuntimeError, "key should be a string type.");
		return PyLong_FromLong(-1);
	}
	
	if (!PyString_Check(group))
	{
		PyErr_SetString(PyExc_RuntimeError, "group should be a string type.");
		return PyLong_FromLong(-1);
	}
	
	if (!PyLong_Check(type))
	{
		PyErr_SetString(PyExc_RuntimeError, "type should be a long type.");
		return PyLong_FromLong(-1);
	}
	
	if (!PyLong_Check(subtype))
	{
		PyErr_SetString(PyExc_RuntimeError, "subtype should be a long type.");
		return PyLong_FromLong(-1);
	}
	
	if (!PyLong_Check(timestamp_sec))
	{
		PyErr_SetString(PyExc_RuntimeError, "timestamp_sec should be a long type.");
		return PyLong_FromLong(-1);
	}
	
	if (!PyLong_Check(timestamp_usec))
	{
		PyErr_SetString(PyExc_RuntimeError, "timestamp_usec should be a long type");
		return PyLong_FromLong(-1);
	}
	
	entry.key = PyString_AsString(key);
	entry.key_count = strlen(entry.key) + 1;
	entry.group = PyString_AsString(key);
	entry.group_count = strlen(entry.group) + 1;
	entry.type = PyInt_AsLong(type);
	entry.subtype = PyInt_AsLong(subtype);
	entry.timestamp_sec = PyInt_AsLong(timestamp_sec);
	entry.timestamp_usec = PyInt_AsLong(timestamp_usec);
    
    switch (entry.subtype)
    {
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_NONE:
    		entry.data = NULL;
    		entry.data_count = 0;
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_STRING:
    		if (!PyString_Check(data))
    		{
    			PyErr_SetString(PyExc_RuntimeError, "data should be a string type.");
    			return PyLong_FromLong(-1);
    		}
    		str = NULL;
    		str = PyString_AsString(data);
    		length = strlen(str) + 1;
    		entry.data = malloc(length);
    		assert(entry.data);
    		memcpy(entry.data, str, length);
    		entry.data_count = length;
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_INT:
    		if (!PyLong_Check(data))
    		{
    			PyErr_SetString(PyExc_RuntimeError, "data should be a long type.");
    			return PyLong_FromLong(-1);
    		}
    		i = 0;
    		i = PyInt_AsLong(data);
    		length = sizeof(i);
    		entry.data = malloc(length);
    		assert(entry.data);
    		memcpy(&entry.data, &i, length);
    		entry.data_count = length;
    	break;
    	case PLAYERC_BLACKBOARD_DATA_SUBTYPE_DOUBLE:
    		if (!PyLong_Check(data))
    		{
    			PyErr_SetString(PyExc_RuntimeError, "data should be a long type.");
    			return PyLong_FromLong(-1);
    		}
    		d = 0.0;
    		d = PyLong_AsDouble(data);
    		length = sizeof(d);
    		entry.data = malloc(length);
    		assert(entry.data);
    		memcpy(&entry.data, &d, length);
    		entry.data_count = length;
    	break;
    	default:
    		entry.data = NULL;
    		entry.data_count = 0;
    	break;
    }
    result = playerc_blackboard_set_entry(self, &entry);
    if (result != 0)
    {
    	return PyLong_FromLong(-1);
    }
    free(entry.data);
    
    return PyLong_FromLong(0);
  }
}

%{
// This code overrides the default swig wrapper. We use a different callback function for the python interface.
// It performs the same functionality, but points the callback function to the new python one. 
playerc_blackboard_t *python_playerc_blackboard_create(playerc_client_t *client, int index)
{
  playerc_blackboard_t *device= playerc_blackboard_create(client, index);
  if (device == NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "Failed to create blackboard");
    return NULL;
  }
  
  device->py_private = Py_BuildValue("({},[],i)",0); // tuple of a dict and a list to store our keys and events in
  
  device->on_blackboard_event = playerc_blackboard_python_on_blackboard_event;
  return device;

}
#undef new_playerc_blackboard
#define new_playerc_blackboard python_playerc_blackboard_create

// We have to clear up the list we created in the callback function.
void python_playerc_blackboard_destroy(playerc_blackboard_t* device)
{
  playerc_device_term(&device->info);
  if (device->py_private != NULL)
  {
  	Py_DECREF((PyObject*)device->py_private);
  }
  free(device);
}
#undef del_playerc_blackboard
#define del_playerc_blackboard python_playerc_blackboard_destroy
%}
