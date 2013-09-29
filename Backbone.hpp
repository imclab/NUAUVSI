enum port_t{
	NO_PORT = 0,
	IMAGES_PULL = 1820,
	IMAGES_PUSH,
	IMAGES_PUB,
	ORTHORECT_PULL,
	ORTHORECT_PUSH,
	GEOREF_PULL,
	GEOREF_PUSH,
	SALIENCY_PULL,
	SALIENCY_PUSH,
	SALIENCY_PUB,
	S_SEG_PULL,
	C_SEG_PULL,
	S_SEG_PUSH,
	C_SEG_PUSH,
	TARGET_PULL,
	TARGET_PUSH,
	TARGET_PUB,
	VERIFIED_PULL,
	VERIFIED_PUB
};

struct msg_t{
	int id;
	bool verified;
	bool workDone;
};
