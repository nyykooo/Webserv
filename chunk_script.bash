(
  echo -en "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n"
  sleep 1
  echo -en "4\r\nWiki\r\n"
  sleep 2
  echo -en "5\r\npedia\r\n"
  sleep 2
  echo -en "E\r\n in\r\nchunks.\r\n"
  sleep 1
  echo -en "0\r\n\r\n"
) | nc 127.0.0.1 8080
