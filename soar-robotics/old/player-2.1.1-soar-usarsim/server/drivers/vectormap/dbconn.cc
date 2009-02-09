#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <cctype>
#include "dbconn.h"
#ifdef HAVE_GEOS
#ifndef GEOS_VERSION_MAJOR
#include <geos_c.h>
#endif
#endif

using namespace std;

bool PostgresConn::Connect(const char* dbname, const char* host, const char* user, const char* password, const char* port)
{
  conn = PQsetdbLogin(host, port, NULL, NULL, dbname, user, password);
  return (PQstatus(conn) != CONNECTION_BAD);
}

bool PostgresConn::Disconnect()
{
  PQfinish(conn);
  conn = NULL;
  //return (PQstatus(conn) == CONNECTION_BAD);
  return true;
}

uint32_t PostgresConn::Text2Bin(const char * text, unsigned char * bin, uint32_t maxlen)
{
    char numbuff[5];
    uint32_t i;

    for (i = 0; i < maxlen; i++)
    {
	if (!(text[0])) break;
	strcpy(numbuff, "0x");
	strncat(numbuff, text, 2);
	bin[i] = strtoul(numbuff, NULL, 0);
	text += 2;
    }
    return i;
}

VectorMapInfoHolder PostgresConn::GetVectorMapInfo(vector<string> layerNames)
{
  // Get the extent in the first query
  string query_string = "SELECT GeometryFromText(astext(extent(geom))) FROM (SELECT geom FROM ";

  for (uint32_t ii=0; ii<layerNames.size(); ++ii)
  {
    if (ii == 0)
    {
      query_string += layerNames[ii];
    }
    else
    {
      query_string += " UNION SELECT geom FROM " + string(layerNames[ii]);
    }
  }
  query_string += ") AS layer_extent;";

  PGresult * res = PQexec(conn, query_string.c_str());

  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    cerr << "Error performing select query on database!" << endl;
    cerr << "No extent value found." << endl;
    cerr << "GetVectorMapInfo() failed" << endl;
  }
 
  uint32_t length = PQgetlength(res, 0, 0);
  uint8_t * wkb = new uint8_t[length];
  assert(wkb);
  length = Text2Bin(PQgetvalue(res, 0, 0), wkb, length);
  BoundingBox extent = BinaryToBBox(wkb, length);
  delete []wkb;
  PQclear(res);

  // Get the srid in the second query
  res = PQexec(conn, "SELECT srid FROM geometry_columns LIMIT 1;");
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    cerr << "Error performing select query on database!" << endl;
    cerr << "No srid value found." << endl;
    cerr << "GetVectorMapInfo() failed" << endl;
  }

  uint32_t srid = atoi(PQgetvalue(res, 0, 0));
  PQclear(res);

  VectorMapInfoHolder info(srid, extent);
  for (uint32_t i=0; i<layerNames.size(); ++i)
  {
    info.layers.push_back(GetLayerInfo(layerNames[i].c_str()));
  }

  return info;
}

LayerInfoHolder PostgresConn::GetLayerInfo(const char* layer_name)
{
  LayerInfoHolder info;

  // Retrieve the extent of the layer in binary form
  const char* query_template = "SELECT GeometryFromText(astext(extent(geom))) AS extent FROM %s;";

  char query_string[MAX_PSQL_STRING];
  memset(query_string, 0, MAX_PSQL_STRING);
  snprintf(query_string, MAX_PSQL_STRING, query_template, layer_name);

  PGresult* res = PQexec(conn, query_string);

  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    cerr << "Error performing select query on database!" << endl;
    cerr << "GetLayerInfo() failed" << endl;
  }

  info.name = layer_name;

  uint32_t length = PQgetlength(res, 0, 0);
  uint8_t * wkb = new uint8_t[length];
  assert(wkb);
  length = Text2Bin(PQgetvalue(res, 0, 0), wkb, length);
  info.extent = BinaryToBBox(wkb, length);
  delete []wkb;

  PQclear(res);

  return info;
}

LayerDataHolder PostgresConn::GetLayerData(const char* layer_name)
{
  // Split into two queries. First get the layer meta-data, then get the feature data.
  // need to get layer name count, layer name, feature count and exteny
  LayerDataHolder data;

  // need to get name count, name, wkb count, wkb
  const char* template_data = "SELECT name, geom, attrib FROM %s ORDER BY id;";
  char query_data[MAX_PSQL_STRING];
  memset(query_data, 0, sizeof(MAX_PSQL_STRING));
  snprintf(query_data, MAX_PSQL_STRING, template_data, layer_name);

  PGresult* res = PQexec(conn, query_data);

  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    cerr << "Error performing select query on database!" << endl;
    cerr << "GetLayerData() data failed, returned NULL" << endl;
  }

  int num_rows = PQntuples(res);
  data.name = layer_name;
  for (int i=0; i<num_rows; ++i)
  {
    FeatureDataHolder * fd = new FeatureDataHolder(string(PQgetvalue(res, i, 0)));
    assert(fd);
    uint32_t length = PQgetlength(res, i, 1);
    uint8_t * wkb = new uint8_t[length];
    assert(wkb);
    length = Text2Bin(PQgetvalue(res, i, 1), wkb, length);
    fd->wkb.assign(wkb, wkb + length);
    delete []wkb;
    fd->attrib = string(PQgetvalue(res, i, 2));

    data.features.push_back(*fd);
    delete fd;
  }

  PQclear(res);

  return data;
}

int PostgresConn::WriteLayerData(LayerDataHolder & data)
{

  const string delcmd = string("DELETE FROM ") + data.name + string(";");
  const string inscmd = string("INSERT INTO ") + data.name + string(" (id, name, geom, attrib) VALUES ($1::integer, $2, $3, $4);");
  PGresult * res;
  const player_vectormap_feature_data_t * feature;
  char numbuff[16];
  const char * params[4];

  res = PQexec(conn, "BEGIN TRANSACTION;");
  if (!res)
  {
    PLAYER_ERROR("Couldn't delete layer features");
    return -1;
  }
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
  {
    PLAYER_ERROR1("%s", PQresultErrorMessage(res));
    PQclear(res);
    return -1;
  }
  PQclear(res);
  PLAYER_WARN1("[%s]", delcmd.c_str());
  res = PQexec(conn, delcmd.c_str());
  if (!res)
  {
    PLAYER_ERROR("Couldn't delete layer features");
    res = PQexec(conn, "ROLLBACK;");
    if (!res)
    {
      PLAYER_ERROR("Couldn't rollback transaction");
      return -1;
    }
    if (PQresultStatus(res) != PGRES_COMMAND_OK) PLAYER_ERROR1("%s", PQresultErrorMessage(res));
    PQclear(res);
    return -1;
  }
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
  {
    PLAYER_ERROR1("%s", PQresultErrorMessage(res));
    PQclear(res);
    res = PQexec(conn, "ROLLBACK;");
    if (!res)
    {
      PLAYER_ERROR("Couldn't rollback transaction");
      return -1;
    }
    if (PQresultStatus(res) != PGRES_COMMAND_OK) PLAYER_ERROR1("%s", PQresultErrorMessage(res));
    PQclear(res);
    return -1;
  }
  PQclear(res);
  for (int i = 0; i < (int)(data.features.size()); i++)
  {
    feature = data.features[i].Convert();
    char * wkb_buff = new char[((feature->wkb_count) * 2) + 1];
    assert(wkb_buff);
    char * ptr = wkb_buff;
    for (uint32_t j = 0; j < (feature->wkb_count); j++)
    {
      snprintf(ptr, 3, "%02x", feature->wkb[j]);
      ptr += 2;
    }
    for (int j = 0; wkb_buff[j]; j++) wkb_buff[j] = toupper(wkb_buff[j]);
    snprintf(numbuff, sizeof numbuff, "%d", i + 1);
    params[0] = numbuff;
    params[1] = (feature->name[0]) ? feature->name : NULL;
    params[2] = wkb_buff;
    params[3] = (feature->attrib[0]) ? feature->attrib : NULL;
    PLAYER_WARN5("[%s] [%s] [%s] [%s] [%s]", inscmd.c_str(), params[0], ((params[1]) ? (params[1]) : ""), params[2], ((params[3]) ? (params[3]) : ""));
    PGresult * res = PQexecParams(conn,
                                  inscmd.c_str(),
                                  4,
                                  NULL,
                                  params,
                                  NULL,
                                  NULL,
                                  0);
    delete []wkb_buff;
    if (!res)
    {
      PLAYER_ERROR("Couldn't insert layer feature to the database");
      res = PQexec(conn, "ROLLBACK;");
      if (!res)
      {
        PLAYER_ERROR("Couldn't rollback transaction");
        return -1;
      }
      if (PQresultStatus(res) != PGRES_COMMAND_OK) PLAYER_ERROR1("%s", PQresultErrorMessage(res));
      PQclear(res);
      return -1;
    }
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
      PLAYER_ERROR1("%s", PQresultErrorMessage(res));
      PQclear(res);
      res = PQexec(conn, "ROLLBACK;");
      if (!res)
      {
        PLAYER_ERROR("Couldn't rollback transaction");
        return -1;
      }
      if (PQresultStatus(res) != PGRES_COMMAND_OK) PLAYER_ERROR1("%s", PQresultErrorMessage(res));
      PQclear(res);
      return -1;      
    }
    PQclear(res);
  }
  res = PQexec(conn, "COMMIT TRANSACTION;");
  if (!res)
  {
    PLAYER_ERROR("Couldn't commit transaction");
    return -1;
  }
  if (PQresultStatus(res) != PGRES_COMMAND_OK)
  {
    PLAYER_ERROR1("%s", PQresultErrorMessage(res));
    PQclear(res);
    return -1;
  }
  PQclear(res);
  return 0;
}

BoundingBox PostgresConn::BinaryToBBox(const uint8_t* wkb, uint32_t length)
{
  BoundingBox res;
  memset(&res, 0, sizeof(BoundingBox));
  if (length == 0)
    return res;
#ifdef HAVE_GEOS
  GEOSGeom polygon;
  polygon = GEOSGeomFromWKB_buf(wkb, length);
  if (polygon == NULL)
  {
    printf("GEOSGeomFromWKB_buf returned NULL!\n");
    return res;
  }
  GEOSGeom linestring = GEOSGetExteriorRing(polygon);
  if (linestring == NULL)
  {
    printf("GEOSGetExteriorRing returned NULL!\n");
    return res;
  }
  GEOSCoordSeq coords = GEOSGeom_getCoordSeq(linestring);
  if (coords == NULL)
  {
    printf("GEOSGeom_getCoordSeq returned NULL!\n");
    return res;
  }

  double xmin = INT_MAX, ymin = INT_MAX;
  double xmax = INT_MIN, ymax = INT_MIN;
  double tempX, tempY = 0;

  for (int ii=0; ii<GEOSGetNumCoordinates(linestring); ++ii)
  {
    GEOSCoordSeq_getX(coords, ii, &tempX);
    GEOSCoordSeq_getY(coords, ii, &tempY);
    if (tempX > xmax)
      xmax = tempX;
    if (tempX < xmin)
      xmin = tempX;
    if (tempY > ymax)
      ymax = tempY;
    if (tempY < ymin)
      ymin = tempY;
  }

  res.x0 = xmin;
  res.y0 = ymin;
  res.x1 = xmax;
  res.y1 = ymax;
  GEOSGeom_destroy(polygon);
#endif
  return res;
}

const player_vectormap_info_t* VectorMapInfoHolder::Convert()
{
  info.srid = srid;
  info.extent.x0 = extent.x0;
  info.extent.y0 = extent.y0;
  info.extent.x1 = extent.x1;
  info.extent.y1 = extent.y1;
  info.layers_count = layers.size();
  if (info.layers) delete [](info.layers);
  info.layers = new player_vectormap_layer_info_t[layers.size()];
  assert(info.layers);
  for (uint32_t ii=0; ii<layers.size(); ++ii)
  {
    info.layers[ii] = *(layers[ii].Convert());
  }
  return &info;
}

VectorMapInfoHolder::~VectorMapInfoHolder()
{
   if (info.layers)
   {
     delete[] info.layers;
   }
}

const player_vectormap_layer_info_t* LayerInfoHolder::Convert()
{
  if (layer_info.name) free(layer_info.name);
  layer_info.name = strdup(name.c_str());
  assert(layer_info.name);
  layer_info.name_count = name.size() + 1;
  layer_info.extent.x0 = extent.x0;
  layer_info.extent.y0 = extent.y0;
  layer_info.extent.x1 = extent.x1;
  layer_info.extent.y1 = extent.y1;
  return &layer_info;
}

const player_vectormap_feature_data_t* FeatureDataHolder::Convert()
{
  if (feature_data.name) free(feature_data.name);
  feature_data.name = strdup(name.c_str());
  assert(feature_data.name);
  feature_data.name_count = name.size() + 1;
  if (feature_data.wkb) delete [](feature_data.wkb);
  feature_data.wkb = new uint8_t[wkb.size()];
  assert(feature_data.wkb);
  feature_data.wkb_count = wkb.size();
  if (feature_data.attrib) free(feature_data.attrib);
  feature_data.attrib = strdup(attrib.c_str());
  assert(feature_data.attrib);
  feature_data.attrib_count = attrib.size() + 1;
  ///TODO: Make more efficient
  for (uint32_t ii=0; ii<wkb.size(); ++ii)
  {
    feature_data.wkb[ii] = wkb[ii];
  }
  return &feature_data;
}

FeatureDataHolder::~FeatureDataHolder()
{
   if (feature_data.name) free(feature_data.name);
   if (feature_data.attrib) free(feature_data.attrib);
   if (feature_data.wkb)
   {
     delete[] feature_data.wkb;
   }
}

const player_vectormap_layer_data_t* LayerDataHolder::Convert()
{
  if (layer_data.name) free(layer_data.name);
  layer_data.name = strdup(name.c_str());
  assert(layer_data.name);
  layer_data.name_count = name.size()+1;
  layer_data.features_count = features.size();
  if (layer_data.features) delete [](layer_data.features);
  layer_data.features = NULL;
  if (layer_data.features_count > 0)
  {
    layer_data.features = new player_vectormap_feature_data_t[layer_data.features_count];
    assert(layer_data.features);
    for (uint32_t ii=0; ii<layer_data.features_count; ++ii)
    {
      layer_data.features[ii] = *(features[ii].Convert());
    }
  }
  return &layer_data;
}

LayerDataHolder::~LayerDataHolder()
{
  if (layer_data.name) free(layer_data.name);
  if (layer_data.features)
  {
     delete[] layer_data.features;
  }
}
