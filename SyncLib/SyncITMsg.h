 /*
  * SyncLib/SyncITMsg.mc
  * Copyright(c) 1999, SyncIt.com  Inc.
  * To compile:
  * MC -C SyncITMsg.mc
  * RC SyncITMsg.rc
  */
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: SYNCIT_OPEN_ERR
//
// MessageText:
//
//  The service could not open the file %1 as input
//
#define SYNCIT_OPEN_ERR                  0x20000001L

//
// MessageId: SYNCIT_CREATE_ERR
//
// MessageText:
//
//  The service could not create the file %1 as output
//
#define SYNCIT_CREATE_ERR                0x20000002L

//
// MessageId: SYNCIT_READ_ERR
//
// MessageText:
//
//  The service could not read from file %1
//
#define SYNCIT_READ_ERR                  0x20000003L

//
// MessageId: SYNCIT_WRITE_ERR
//
// MessageText:
//
//  The service could not write to file %1
//
#define SYNCIT_WRITE_ERR                 0x20000004L

//
// MessageId: SYNCIT_FILE_ERR
//
// MessageText:
//
//  An error occurred while accessing the file %1
//
#define SYNCIT_FILE_ERR                  0x20000005L

//
// MessageId: SYNCIT_CLOSE_ERR
//
// MessageText:
//
//  The service could not close the file %1
//
#define SYNCIT_CLOSE_ERR                 0x20000006L

//
// MessageId: SYNCIT_REGKEY_ERR
//
// MessageText:
//
//  An error occurred while accessing the registry key %1
//
#define SYNCIT_REGKEY_ERR                0x20000007L

//
// MessageId: SYNCIT_CREATEKEY_ERR
//
// MessageText:
//
//  The service could not create the registry key %1
//
#define SYNCIT_CREATEKEY_ERR             0x20000008L

//
// MessageId: SYNCIT_OPENKEY_ERR
//
// MessageText:
//
//  The service could not open the registry key %1
//
#define SYNCIT_OPENKEY_ERR               0x20000009L

//
// MessageId: SYNCIT_REGQUERY_ERR
//
// MessageText:
//
//  The service could not query the registry value %1\\%2
//
#define SYNCIT_REGQUERY_ERR              0x2000000AL

//
// MessageId: SYNCIT_NETCONNECT_ERR
//
// MessageText:
//
//  The service could not connect to the network computer %1
//
#define SYNCIT_NETCONNECT_ERR            0x2000000BL

//
// MessageId: SYNCIT_HOSTNAME_ERR
//
// MessageText:
//
//  The service could not locate the network computer %1
//
#define SYNCIT_HOSTNAME_ERR              0x2000000CL

//
// MessageId: SYNCIT_NETREAD_ERR
//
// MessageText:
//
//  The service could not read data from the network computer %1
//
#define SYNCIT_NETREAD_ERR               0x2000000DL

//
// MessageId: SYNCIT_NETWRITE_ERR
//
// MessageText:
//
//  The service could not write data to the network computer %1
//
#define SYNCIT_NETWRITE_ERR              0x2000000EL

//
// MessageId: SYNCIT_NETCLOSE_ERR
//
// MessageText:
//
//  The service could not gracefully disconnect from the network computer %1
//
#define SYNCIT_NETCLOSE_ERR              0x2000000FL

//
// MessageId: SYNCIT_HTTP_ERR
//
// MessageText:
//
//  A %1 %2 error occurred while connecting to the SyncIT web server.
//
#define SYNCIT_HTTP_ERR                  0x20000010L

//
// MessageId: SYNCIT_SERVER_ERR
//
// MessageText:
//
//  A %1 error occurred while synchronizing with the SyncIT web server.
//
#define SYNCIT_SERVER_ERR                0x20000011L

//
// MessageId: SYNCIT_PROTOCOL_ERR
//
// MessageText:
//
//  A protocol error %1 occurred while synchronizing with the SyncIT web server.
//
#define SYNCIT_PROTOCOL_ERR              0x20000012L

//
// MessageId: SYNCIT_NOT_DIALED_IN
//
// MessageText:
//
//  Your computer may not be dialed into the Internet.
//
#define SYNCIT_NOT_DIALED_IN             0x20000013L

