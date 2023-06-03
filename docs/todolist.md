## todo list

- [x] payloadTooLarge 처리
- [x] max body size 처리
- [ ] `std::cout` 정리
- [ ] *.conf 파일에 맞추어 디렉토리 파일 정리 / 반대로 *.conf 파일도 정리

## test list

### 기본 기능 테스트
- [ ] GET 요청
- [ ] POST 요청
- [ ] DELETE 요청

### cgi 요청
- [ ] GET 요청
- [ ] POST 요청

### 기본 체크 사항
- [ ] 여러 페이지의 접속(끊김 없이 가능한지)
- [ ] `kevent()` 몇 번 사용했는지?
- [ ] `write()`나 `read()` 호출 이후 실패시 `errno` 확인하지는 않는 지
