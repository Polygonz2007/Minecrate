#ifndef BLOCK_H


// DEFINE BLOCK STRUCT
struct {
	int type;
} block;


// DEFINE BLOCK NAMES FOR EACH TYPE
const char block_names[4][8] = {
  "Air",
  "Grass",
  "Dirt",
  "Sand"
};

// DEFINE BLOCK HARDNESS(ES)
const int block_hardness[4]{
	0, 2, 2, 2
};

#endif