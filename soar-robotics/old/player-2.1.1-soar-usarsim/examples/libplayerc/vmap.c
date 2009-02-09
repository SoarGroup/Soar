#include <libplayerc/playerc.h>

void PrintMapInfo(playerc_vectormap_t* vmap);
void PrintLayerInfo(playerc_vectormap_t* vmap);
void PrintLayerData(playerc_vectormap_t* vmap);
void PrintFeatureData(playerc_vectormap_t* vmap);

int main()
{
  playerc_client_t* client = NULL;
  playerc_vectormap_t* vmap = NULL;
  GEOSGeom g;

  printf("Creating client\n");
  client = playerc_client_create(NULL, "localhost", 6665);
  if (0 != playerc_client_connect(client))
  {
          printf("Error connecting client\n");
          return -1;
  }

  printf("Creating vectormap\n");
  vmap = playerc_vectormap_create(client, 0);
  if (vmap == NULL)
  {
          printf("vmap is NULL\n");
          return -1;
  }

  printf("Subscribing\n");
  if (playerc_vectormap_subscribe(vmap, PLAYER_OPEN_MODE))
  {
          printf("Error subscribing\n");
          return -1;
  }

  assert(vmap != NULL);

  printf("Getting map info\n");
  if (playerc_vectormap_get_map_info(vmap))
  {
          printf("Error getting info\n");
          return -1;
  }

  PrintMapInfo(vmap);
  PrintLayerInfo(vmap);

  printf("Getting layer data\n");
  if (playerc_vectormap_get_layer_data(vmap, 0))
  {
          printf("Error getting layer data\n");
  }

  PrintMapInfo(vmap);
  PrintLayerInfo(vmap);
  PrintLayerData(vmap);

  printf("Getting feature data\n");
  g = playerc_vectormap_get_feature_data(vmap, 0, 0);
  if (g == NULL)
  {
    printf("Error getting feature data\n");
  }

  PrintMapInfo(vmap);
  PrintLayerInfo(vmap);
  PrintLayerData(vmap);
  PrintFeatureData(vmap);

  printf("\n");
  printf("Unsubscribing\n");
  playerc_vectormap_unsubscribe(vmap);
  return 0;
}

void PrintMapInfo(playerc_vectormap_t* vmap)
{
  player_extent2d_t extent = vmap->extent;
  printf("MapInfo\n");
  printf("srid = %d\nlayer_count = %d\n", vmap->srid, vmap->layers_count);
  printf("extent = (%f %f, %f %f)\n", extent.x0, extent.y0, extent.x1, extent.y1);
}

void PrintLayerInfo(playerc_vectormap_t* vmap)
{
  player_extent2d_t extent = vmap->layers_info[0]->extent;
  printf("LayerInfo\n");
  printf("extent = (%f %f, %f %f)\n", extent.x0, extent.y0, extent.x1, extent.y1);
  printf("name = %s\n", vmap->layers_info[0]->name);
}

void PrintLayerData(playerc_vectormap_t* vmap)
{
  printf("LayerData\n");
  printf("feature count = %d\n", vmap->layers_data[0]->features_count);
}

void PrintFeatureData(playerc_vectormap_t* vmap)
{
  printf("FeatureData\n");
  printf("wkb count = %d\n", vmap->layers_data[0]->features[0].wkb_count);
  printf("name = %s\n", vmap->layers_data[0]->features[0].name);
}
