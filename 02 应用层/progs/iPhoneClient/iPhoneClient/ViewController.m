//
//  ViewController.m
//  iPhoneClient
//
//  Created by YangHan on 2018/9/12.
//  Copyright © 2018年 YangHan. All rights reserved.
//

#import "ViewController.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

@interface ViewController ()
@property (weak, nonatomic) IBOutlet UIButton *button;
@property (weak, nonatomic) IBOutlet UILabel *stateLabel;
@property (weak, nonatomic) IBOutlet UITextField *ipTextField;
@property (weak, nonatomic) IBOutlet UITextField *portTextField;
@property (nonatomic,assign) pthread_t readDataThread;
@property (nonatomic,assign) int  sockfd;
@end

@implementation ViewController

- (IBAction)didTapButton:(UIButton *)sender {
    
    self.button.enabled = NO;
    
    void (^ErrorBlock)(void) = ^{
        [self.view endEditing:YES];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            self.stateLabel.text = @"Nothing to show";
            self.button.enabled = YES;
        });
    };
    
    NSString *httpMsg = [NSString stringWithFormat:@"GET index.html HTTP/1.1\n"];
    const char  *httpMsgCstr = [httpMsg cStringUsingEncoding:kCFStringEncodingUTF8];
    
    if (self.ipTextField.text.length == 0) {
        self.stateLabel.text = @"请输入 Ip 地址";
        ErrorBlock();
        return;
    }
    if (self.portTextField.text.length == 0) {
        self.stateLabel.text = @"请输入 port";
        ErrorBlock();
        return;
    }
    
    const char *ipAddr = [self.ipTextField.text cStringUsingEncoding:kCFStringEncodingUTF8];
    unsigned short port = (unsigned short)[self.portTextField.text intValue];
    
    self.stateLabel.text = @"开始建立 TCP 连接...";
    
    if ((self.sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        self.stateLabel.text = [NSString stringWithFormat:@"Create socket error! : %s", strerror(errno)];
        ErrorBlock();
        return;
    }
    
    struct sockaddr_in svraddr;
    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipAddr, &svraddr.sin_addr) < 0) {
        self.stateLabel.text = @"非法的 ip 地址";
        ErrorBlock();
        return;
    }
    
    if (connect(self.sockfd, (struct sockaddr *)&svraddr, sizeof(svraddr)) < 0) {
        self.stateLabel.text = [NSString stringWithFormat:@"Connect error! : %s", strerror(errno)];
        ErrorBlock();
        return;
    }
    
    self.stateLabel.text = [NSString stringWithFormat:@"[%s:%d] TCP 连接建立成功!", ipAddr, port];
    [self.view endEditing:YES];
    
    ssize_t nWritten;
    size_t msgLen = strlen(httpMsgCstr);
    if ((nWritten = write(self.sockfd, httpMsgCstr, msgLen)) != msgLen) {
        self.stateLabel.text = [NSString stringWithFormat:@"Connect error! : %s", strerror(errno)];
        ErrorBlock();
        return;
    }
    
    int err;
    if ((err = pthread_create(&_readDataThread, NULL, backgroundBusyLoop, (__bridge void *)(self))) < 0) {
        errno = err;
        self.stateLabel.text = [NSString stringWithFormat:@"Connect error! : %s", strerror(errno)];
        ErrorBlock();
        return;
    }
}

void * backgroundBusyLoop(void *arg) {
    ViewController *vc = (__bridge ViewController *)arg;
    while (1) {
        ssize_t nRead;
        char readBuffer[1024];
        if ((nRead = read(vc.sockfd, readBuffer, 1024)) < 0) {
            if (errno != EAGAIN) {
                vc.stateLabel.text = [NSString stringWithFormat:@"Read data fail! :%s", strerror(errno)];
                return NULL;
            } else {
                vc.stateLabel.text = [NSString stringWithFormat:@"服务器没有回传数据"];
            }
        } else if (nRead == 0) {
            vc.stateLabel.text = [NSString stringWithFormat:@"%@ 关闭了套接字, read EOF!", vc.ipTextField.text];
            vc.stateLabel.text = @"Nothing to show";
            vc.button.enabled = YES;
            close(vc.sockfd);
            return NULL;
        }
        readBuffer[nRead] = 0;
        vc.stateLabel.text = [NSString stringWithFormat:@"%zd 字节，%s",nRead, readBuffer];
        [vc.view setNeedsDisplay];
        return NULL;
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.stateLabel.text = @"Nothing to show";
    [self.button setTitle:@"建立 TCP 连接" forState:UIControlStateNormal];
    
    self.ipTextField.text = @"10.10.5.165";
    self.portTextField.text = @"5000";
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self.ipTextField resignFirstResponder];
    [self.portTextField resignFirstResponder];
}

@end
