## Enable logs while runing project
1. ./ns3 configure --enable-logs
1. export NS_LOG=<LogComponentName>
1. ./ns3 run project name


/*
* Create a NodeContainer for all the UEs
*/
NodeContainer allSlUesContainer;
allSlUesContainer.Create(numberOfNodes);

/*
* Assign mobility to the UEs.
*/
MobilityHelper mobility;
mobility.Install (allSlUesContainer);

```cpp

    template <typename T>
    Seq<T> makeSeq(asn_TYPE_descriptor_t * def) {
        return Seq<T>(def);
    }
typedef ssize_t ber_tlv_len_t;

typedef struct asn_struct_ctx_s {
	short phase;		/* Decoding phase */
	short step;		/* Elementary step of a phase */
	int context;		/* Other context information */
	void *ptr;		/* Decoder-specific stuff (stack elements) */
	ber_tlv_len_t left;	/* Number of bytes left, -1 for indefinite */
} asn_struct_ctx_t;

/* ItsPduHeader */
typedef struct ItsPduHeader {
	long	 protocolVersion;
	long	 messageID;
	StationID_t	 stationID;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ItsPduHeader_t;

typedef long	 StationType_t;
/* GenerationDeltaTime */
typedef long	 GenerationDeltaTime_t;
/* CoopAwareness */
typedef struct CoopAwareness {
	GenerationDeltaTime_t	 generationDeltaTime;
	CamParameters_t	 camParameters;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CoopAwareness_t;
    /* CAM */
typedef struct CAM {
	ItsPduHeader_t	 header;
	CoopAwareness_t	 cam;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CAM_t;

void send_cam_pkt() {

    auto cam = makeSeq(CAM);

    setField(cam->header.messageId, FIX_CAMID);
    setField(cam->header.protocolVersion , 2);
    setField(cam->header.stationId, m_station_id); // get it from the previous emergenccy stations

    Ptr<Packet> packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());

    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = CA_PORT;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = TSB;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt =0;
    dataRequest.GNMaxRepInt=0;
    dataRequest.GNMaxLife = 1;
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x02; // Store carry foward: no - Channel offload: no - Traffic Class ID: 2
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;
    m_btp->sendBTP(dataRequest);
}
```