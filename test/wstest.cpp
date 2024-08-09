#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fmp/ws.h>

using namespace rai;
using namespace kv;
using namespace ds;
using namespace fmp;

static const char *
get_arg( int argc, char *argv[], int b, const char *f,
         const char *def ) noexcept
{
  for ( int i = 1; i < argc - b; i++ ) {
    if ( ::strcmp( f, argv[ i ] ) == 0 ) {
      if ( b == 0 || argv[ i + b ][ 0 ] != '-' )
        return argv[ i + b ];
      return def;
    }
  }
  return def; /* default value */
}

static const char ws_key[] = "dGhlIHNhbXBsZSBub25jZQ==",
                  ws_acc[] = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";

struct MyClientCB : public HttpClientCB, public EvTimerCallback {
  HttpClient & client;
  const char * host,
             * sym,
             * api;
  MyClientCB( HttpClient &cl ) : client( cl ), host( 0 ), sym( 0 ), api( 0 ) {}

  virtual void on_http_msg( HttpRsp &rsp ) noexcept;
  virtual void on_switch( HttpRsp &rsp ) noexcept;
  virtual void on_ws_msg( WSMsg &ws ) noexcept;
  virtual bool timer_cb( uint64_t, uint64_t ) noexcept;
};

void
MyClientCB::on_http_msg( HttpRsp &rsp ) noexcept
{
  printf( "%.*s\n", (int) rsp.content_length, rsp.data );
}

void
MyClientCB::on_switch( HttpRsp &rsp ) noexcept
{
  printf( "switch proto\n" );
  if ( ::memcmp( ws_acc, rsp.wsacc, rsp.wsacclen ) == 0 &&
       rsp.wsacclen == sizeof( ws_acc ) - 1 )
    printf( "matches\n" );

  VarHT ht;
  this->client.send_request( 
    "{ \"event\": \"login\", \"data\": { \"apiKey\": \"@(apikey)\" }}",
    ht.add( Pair( "apikey", 6, this->api, ::strlen( this->api ) ) ) );
  this->client.poll.timer.add_timer_seconds( *this, 1, 1, 1 );
}

void
MyClientCB::on_ws_msg( WSMsg &ws ) noexcept
{
  printf( "on msg %u\n", ws.frame.opcode );
  printf( "%.*s\n", (int) ws.len, (char *) ws.data );
}

bool
MyClientCB::timer_cb( uint64_t, uint64_t ) noexcept
{
  VarHT ht;
  printf( "subscribe\n" );
  this->client.send_request( 
    "{ \"event\": \"subscribe\", \"data\": { \"ticker\": \"@(symbol)\" }}",
    ht.add( Pair( "symbol", 6, this->sym, ::strlen( this->sym ) ) ) );
  return false;
}

int
main( int argc, char *argv[] )
{ 
  SignalHandler sighndl;
  EvPoll        poll;
  SSL_Context   ctx;

  if ( get_arg( argc, argv, 0, "-h", NULL ) != NULL ) {
    printf( "%s "
    "[-c addr] [-p port] [-f cert.pem] [-k key.pem] [-a ca.crt] [-d /etc/ssl]\n"
    "    -n           : no verify cert\n"
    "    -c address   : connect client to address\n"
    "    -u uri       : uri for client get\n"
    "    -a apikey    : api key for client\n"
    "    -p port      : port number of addr\n"
    "    -f cert.pem  : certificate pem file to use\n"
    "    -k key.pem   : key pem file\n"
    "    -a ca.crt    : certificate authority pem file\n"
    "    -d /dir      : local cache of certificates\n",
    argv[ 0 ] );
    return 1;
  }

  bool is_client = true;
  int  port      = atoi( get_arg( argc, argv, 1, "-p", "443" ) );

  if ( get_arg( argc, argv, 0, "-n", NULL ) == NULL ) {
    const char * crt_file = get_arg( argc, argv, 1, "-f", NULL ),
               * key_file = get_arg( argc, argv, 1, "-k", NULL ),
               * ca_file  = get_arg( argc, argv, 1, "-a", NULL ),
               * ca_dir   = get_arg( argc, argv, 1, "-d", NULL );
    SSL_Config cfg( crt_file, key_file, ca_file, ca_dir, is_client, false );
    if ( ! ctx.init_config( cfg ) )
      return 1;
  }
  else {
    SSL_Config cfg( NULL, NULL, NULL, NULL, is_client, false );
    cfg.no_verify = true;
    if ( ! ctx.init_config( cfg ) )
      return 1;
  }
  poll.init( 5, false );

  HttpClient client( poll );
  MyClientCB mycb( client );

  mycb.host = get_arg( argc, argv, 1, "-c", "127.0.0.1" ),
  mycb.sym  = get_arg( argc, argv, 1, "-s", "AMD" ),
  mycb.api  = get_arg( argc, argv, 1, "-a", NULL );

  if ( mycb.api == NULL ) {
    fprintf( stderr, "need api key\n" );
    return 1;
  }
  VarHT ht;
  if ( EvTcpConnection::connect( client, mycb.host, port,
                            DEFAULT_TCP_CONNECT_OPTS | OPT_CONNECT_NB ) != 0 )
    return 1;

  client.init_ssl_connect( ctx );
  client.cb = &mycb;
  if ( get_arg( argc, argv, 0, "-g", NULL ) != NULL ) {
    client.send_request(
      "GET /api/v3/quote/@(symbol)?apikey=@(apikey) HTTP/1.1\r\n"
      "Host: @(host)\r\n"
      "\r\n",
      ht.add( Pair( "host"   , 4, mycb.host , ::strlen( mycb.host ) ) )
        .add( Pair( "symbol" , 6, mycb.sym  , ::strlen( mycb.sym ) ) )
        .add( Pair( "apikey" , 6, mycb.api  , ::strlen( mycb.api ) ) )
    );
  }
  else if ( get_arg( argc, argv, 0, "-w", NULL ) != NULL ) {
    client.send_request(
      "GET / HTTP/1.1\r\n"
      "Host: @(host)\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Key: @(wskey)\r\n"
      "Sec-WebSocket-Version: 13\r\n"
      "Sec-WebSocket-Protocol: @(proto)\r\n"
      "\r\n",
      ht.add( Pair( "host" , 4, mycb.host  , ::strlen( mycb.host ) ) )
        .add( Pair( "wskey", 5, ws_key     , sizeof( ws_key ) - 1 ) )
        .add( Pair( "proto", 5, "websocket", 9 ) )
    );
  }
  else {
    static char demo[] = "demo.piesocket.com", 
                uri [] = "/v3/channel_123",
                api [] = "VCXCEuvhGcBDP7XhiJJUDvR1e1D3eiVjgZ9VRiaV&notify_self";
    client.send_request(
      "GET @(uri)?api_key=@(api) HTTP/1.1\r\n"
      "Host: @(host)\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Key: @(wskey)\r\n"
      "Sec-WebSocket-Version: 13\r\n"
      "\r\n",
      ht.add( Pair( "host" , 4, demo       , sizeof( demo ) - 1 ) )
        .add( Pair( "wskey", 5, ws_key     , sizeof( ws_key ) - 1 ) )
        .add( Pair( "uri"  , 3, uri        , sizeof( uri ) - 1 ) )
        .add( Pair( "api"  , 3, api        , sizeof( api ) - 1 ) )
        .add( Pair( "proto", 5, "piesocket", 9 ) )
    );
  }
  /*poll.timer.add_timer_seconds( client.fd, 1, 1, 10 );*/
  sighndl.install();
  for ( int idle_count = 0; ; ) {
    /* loop 5 times before quiting, time to flush writes */
    if ( poll.quit >= 5 && idle_count > 0 )
      break;
    /* dispatch network events */
    int idle = poll.dispatch();
    if ( idle == EvPoll::DISPATCH_IDLE )
      idle_count++;
    else
      idle_count = 0;
    /* wait for network events */ 
    poll.wait( idle_count > 2 ? 100 : 0 );
    if ( sighndl.signaled )
      poll.quit++;
  }
  return 0;
}
