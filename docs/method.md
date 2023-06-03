
	
	바디는 요청의 목적에 따라 생략 가능합니다. 
    하지만 요청 바디가 필요한 경우, Content-Length와 Content-Type 헤더를 함께 사용하여 요청 바디의 크기와 형식을 명시해야 합니다.

##	1. GET
```
	GET /api/users HTTP/1.1

	Host: example.com
	Accept: application/json-->
```

## 	2. POST
```
	POST /api/users HTTP/1.1
	
    Host: example.com
	Content-Type: application/json
	{
	  "username": "johndoe",
	  "password": "secret"
	}

```



###	2-1. POST-Chunked
```
	POST /upload HTTP/1.1

	Host: example.com
	Content-Type: text/plain
	Transfer-Encoding: chunked

	7   // text의 길이
	Hello,
	6  // text의 길이 
	world!
	0 // 0은 마무리를 알림

```
 

##	3. DELETE
```
	DELETE /api/users/123 HTTP/1.1
    
	Host: example.com
```
