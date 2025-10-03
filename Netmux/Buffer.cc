#include"Buffer.h"
#include<errno.h>
#include<sys/uio.h>
#include<unistd.h>
/*************
public method
*************/
ssize_t Buffer::readFd(int fd, int* saveErrno){
    char extrabuf[65536]={0};// the memory of stack 64k
    struct iovec vec[2];
    const size_t writable=writableBytes();
    vec[0].iov_base=begin()+writerIndex_;
    vec[0].iov_len=writable;
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof(extrabuf);
    const int iovcnt=(writable<sizeof(extrabuf));
    const ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0){
        *saveErrno=errno;
    }else if(n<=writable){
        writerIndex_+=n;
    }else{
        writerIndex_=buffer_.size();
        append(extrabuf,n-writable);
    }
    return n;
}
//put user buffer into kernel buffer
ssize_t Buffer::writeFd(int fd, int* saveErrno){
    ssize_t n=::write(fd,peek(),readableBytes());
    if(n<0)*saveErrno=errno;
    return n;
}
/*************
private method
*************/
void Buffer::makeSpace(size_t len){
    if(writableBytes()+prependableBytes()<kCheapPrepend+len){
        buffer_.resize(writerIndex_+len);
    }else{
        size_t readable=readableBytes();
        std::copy(begin()+readerIndex_,begin()+writerIndex_,begin()+kCheapPrepend);
        readerIndex_=kCheapPrepend;
        writerIndex_=readerIndex_+readable;
    }
}
