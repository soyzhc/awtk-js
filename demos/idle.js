
var count = 0;
TIdle.add(function() {
  print(count);
  if(count < 10) {
    count += 1;
    return TRet.REPEAT;
  } else {
    return TRet.OK;
  }
});
