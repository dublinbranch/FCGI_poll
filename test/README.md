# Testing FCGX

Improved FastCGI library tests. Pay attention to its testing ports, e.g. **2006**.

In order to build tests and running them, use typical command:

	rm -rf build && mkdir -p build && cd build && cmake .. -DModifiedFastCGI_NGINX=1 && make && ctest

### NGINX

Don't forget to add to your **nginx** configuration file if you choose using NGINX as FastCGI server provider. You can invoke its help for further info:

        ./nginx_fastcgi_test --log_level=message -- --help

        Usage ./nginx_fastcgi_test [--log_level=message] -- [--help] [--nginx_location=NGINX configured location at nginx.conf] [--fastcgi_server=ip for testing server] [--fastcgi_port=port for testing server]

        All those commandline arguments are optional provided NGINX is properly configured and running:

          By default, --nginx_location="http://0.0.0.0/testingFCGX

          By default, --fastcgi_server=0.0.0.0

          By default, --fastcgi_port=2006

        Example of NGINX configuration:

            location /testingFCGX  {

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

