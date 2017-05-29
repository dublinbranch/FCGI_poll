# Modified FastCGI library

The function **is_af_unix_keeper** has been removed, is no more usefull because the socket is now marked as *NONBLOCKING WITH TIMEOUT*, IE after timeval amount of time the read will return the *EAGAIN* error code (so will not remain struct waiting)

This mod allow the code to **DO NOT USE neither SELECT** (old, buggy, slow, prone to corrupt you stack if using > 1024 FD) or poll / epoll (less kernel call / context switch bla bla bla)

As well it was removed the **select inside OS_Close** (again also here the read do not block forever)

## Testing FCGX

Pay attention to its testing ports, e.g. **2006** or **9000**, just in case they are in use. There are two possible kind of tests:

### StandAlone

In order to build tests and running them, use typical command:

#### for humans

Choose the number of threads for *make -j XXXX*:

	rm -rf build && mkdir -p build && cd build && cmake .. && make -j6 && ctest

#### for machines

Supposedly faster and autochosen number of threads:

	rm -rf build && mkdir -p build && cd build && cmake .. -G "Ninja" && ninja && ctest

#### Hints

Don't forget to make golang binary utility **[fastcgi-server](https://github.com/beberlei/fastcgi-serve)** accessible (building with *cmake* will try to install) to be internally invoked by this test:

	./standalone_fastcgi_test --log_level=message -- --help

        Usage ./standalone_fastcgi_test [--log_level=message] -- [--help] [--webserver0_location=http webserver automatically launched by this test] [--webserver1_location=http webserver automatically launched by this test] [--fastcgi_server=ip for testing server] [--fastcgi0_port=port for testing server] [--fastcgi1_port=port for testing server] [--tool=fastcgi-serve full path if needed]

        All those commandline arguments are optional provided golang testing binary fastcgi-serve tool is properly installed (done by cmake or by 'go get github.com/beberlei/fastcgi-serve'):

          By default, --websever0_location=0.0.0.0:9000

          By default, --websever1_location=0.0.0.0:9001

          By default, --fastcgi_server=0.0.0.0

          By default, --fastcgi0_port=2006

          By default, --fastcgi1_port=2007

          By default, --tool=fastcgi-serve

        So fastcgi-serve internally usage invoked by this test will be:

            fastcgi-serve --listen=0.0.0.0:9000 --server=0.0.0.0 --server-port=2006

        Further info at https://github.com/beberlei/fastcgi-serve

### NGINX

In order to build tests and running them, use typical command:

#### for humans

Choose the number of threads for *make -j XXXX*:

	rm -rf build && mkdir -p build && cd build && cmake .. -DModifiedFastCGI_NGINX=1 && make -j6 && ctest

#### for machines

Supposedly faster and autochosen number of threads:

	rm -rf build && mkdir -p build && cd build && cmake .. -G "Ninja" -DModifiedFastCGI_NGINX=1 && ninja && ctest

#### Hints

Don't forget to add to your **nginx** configuration file if you choose using NGINX as FastCGI server provider. You can invoke its help for further info:

        ./nginx_fastcgi_test --log_level=message -- --help

        Usage ./nginx_fastcgi_test [--log_level=message] -- [--help] [--nginx0_location=NGINX configured location at nginx.conf] [--nginx1_location=NGINX configured location at nginx.conf] [--fastcgi_server=ip for testing server] [--fastcgi0_port=port for testing server] [--fastcgi1_port=port for testing server]


        All those commandline arguments are optional provided NGINX is properly configured and running:

          By default, --nginx0_location=http://0.0.0.0/testingFCGX0

          By default, --nginx1_location=http://0.0.0.0/testingFCGX1

          By default, --fastcgi_server=0.0.0.0

          By default, --fastcgi0_port=2006

          By default, --fastcgi1_port=2007

        Example of NGINX configuration:

            location /testingFCGX0  {

                    fastcgi_pass   0.0.0.0:2006;
                    fastcgi_connect_timeout 5h;
                    fastcgi_read_timeout 5h;

                    fastcgi_param  QUERY_STRING       $query_string;
                    fastcgi_param  REQUEST_METHOD     $request_method;
                    fastcgi_param  CONTENT_TYPE       $content_type;
                    fastcgi_param  CONTENT_LENGTH     $content_length;
                    fastcgi_param  REQUEST            $request;
                    fastcgi_param  REQUEST_BODY       $request_body;
                    fastcgi_param  REQUEST_URI        $request_uri;
                    fastcgi_param  DOCUMENT_URI       $document_uri;
                    fastcgi_param  DOCUMENT_ROOT      $document_root;
                    fastcgi_param  SERVER_PROTOCOL    $server_protocol;
                    fastcgi_param  REMOTE_ADDR        $remote_addr;
                    fastcgi_param  REMOTE_PORT        $remote_port;
                    fastcgi_param  SERVER_ADDR        $server_addr;
                    fastcgi_param  SERVER_PORT        $server_port;
                    fastcgi_param HTTP_REFERER        $http_referer;
                    fastcgi_param SCHEME              $scheme;
              }

             location /testingFCGX1  {

                    fastcgi_pass   0.0.0.0:2007;
                    fastcgi_connect_timeout 5h;
                    fastcgi_read_timeout 5h;

                    fastcgi_param  QUERY_STRING       $query_string;
                    fastcgi_param  REQUEST_METHOD     $request_method;
                    fastcgi_param  CONTENT_TYPE       $content_type;
                    fastcgi_param  CONTENT_LENGTH     $content_length;
                    fastcgi_param  REQUEST            $request;
                    fastcgi_param  REQUEST_BODY       $request_body;
                    fastcgi_param  REQUEST_URI        $request_uri;
                    fastcgi_param  DOCUMENT_URI       $document_uri;
                    fastcgi_param  DOCUMENT_ROOT      $document_root;
                    fastcgi_param  SERVER_PROTOCOL    $server_protocol;
                    fastcgi_param  REMOTE_ADDR        $remote_addr;
                    fastcgi_param  REMOTE_PORT        $remote_port;
                    fastcgi_param  SERVER_ADDR        $server_addr;
                    fastcgi_param  SERVER_PORT        $server_port;
                    fastcgi_param HTTP_REFERER        $http_referer;
                    fastcgi_param SCHEME              $scheme;
              }


