Implementations de hoje

Bom, primeiramente, estados do processmaneto de um request do client.

Cada client passa a ter um atributo referente ao estado do tratamento da seu Request atual. Novo enum na classe client.hpp.

Depois, no method de server que tratamos os clients existentes implementamos o uso desses estados.

Passei a removeSlashes par aum escopo global, utils, para poder utiliza-la.

Importante tb mencionar que usamos um objeto temporario da httpResponse pra lidar com a busca pelos paths dos files 
consideransdo a logica dos location blocks.

Um depuracao visual de um caso de GET de um grande file.

1. Comecando jah pela StartServer, eh e metodo que tem o loop da epoll_wait, que aguarda conexoes, dados, etc.. e tbm tem o metodo handle events. Alem da lookfortimeOuts.
2. Um cliente conecta com http://ipdoserver:8080/video.mp4, isso gera um request do tipo 

GET /video.mp4 HTTP/1.1
Host: localhost:8080
User-Agent: curl/7.68.0
