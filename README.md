프로젝트 개요
=

이 프로젝트는 Boost ASIO 를 이용한 EchoServer로 윈도우즈 환경 VS2015 커뮤니티 에디션에서 작성됐습니다.

/////////////////////////////////////////////////////////////////////////////

bin

빌드된 결과물인 TestClient 와 TestServer 가 존재합니다.


실행 시 

TestClient 127.0.0.1 9999
TestClient 9999 9999

형태로 실행해주시기 바랍니다.

/////////////////////////////////////////////////////////////////////////////

NetClient

클라이언트 본체로 기본적인 기능을 포함하며 비공통 부분은 콜백형태로 동작하게 돼 있습니다.

/////////////////////////////////////////////////////////////////////////////

NetServer

서버 본체로 기본적인 기능을 포함하며 비공통 부분은 콜백형태로 동작하게 돼 있습니다.

/////////////////////////////////////////////////////////////////////////////

NetSession

네트워크 세션을 관리합니다. NetClient와 NetServer 가 공통으로 사용하고 있습니다.

/////////////////////////////////////////////////////////////////////////////

Shared

Server/Client 공통으로 사용되는 부분이 정의 돼 있습니다.

/////////////////////////////////////////////////////////////////////////////

TestClient

라이브러리를 이용해서 만든 테스트 클라이언트 입니다.


/////////////////////////////////////////////////////////////////////////////

TestServer

라이브러리를 이용해서 만든 테스트 서버 입니다.

/////////////////////////////////////////////////////////////////////////////
