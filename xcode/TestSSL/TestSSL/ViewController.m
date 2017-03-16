//
//  ViewController.m
//  TestSSL
//
//  Created by 曹福昂 on 17/3/16.
//  Copyright © 2017年 曹福昂. All rights reserved.
//

#import "ViewController.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    mSocket = [[GCDAsyncSocket alloc] initWithDelegate:self delegateQueue:dispatch_queue_create("cavan", nil)];
    
    NSError *err = nil;
    if ([mSocket connectToHost:@"192.168.10.19" onPort:9982 error:&err]) {
        NSLog(@"Successfull");
    } else {
        NSLog(@"Failed: err = %@", err);
    }
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (nullable dispatch_queue_t)newSocketQueueForConnectionFromAddress:(NSData *)address onSocket:(GCDAsyncSocket *)sock {
    NSLog(@"newSocketQueueForConnectionFromAddress");
    return nil;
}

- (void)socket:(GCDAsyncSocket *)sock didAcceptNewSocket:(GCDAsyncSocket *)newSocket {
    NSLog(@"didAcceptNewSocket");
}

- (void)socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(uint16_t)port {
    NSLog(@"didConnectToHost");
    
    // 配置 SSL/TLS 设置信息
    NSMutableDictionary *settings = [NSMutableDictionary dictionaryWithCapacity:3];
    //允许自签名证书手动验证
    [settings setObject:@YES forKey:GCDAsyncSocketManuallyEvaluateTrust];
    //GCDAsyncSocketSSLPeerName
    [settings setObject:@"www.jwaoo.com" forKey:GCDAsyncSocketSSLPeerName];
    // 如果不是自签名证书，而是那种权威证书颁发机构注册申请的证书
    // 那么这个settings字典可不传。
    [sock startTLS:settings]; // 开始SSL握手
}

- (void)socket:(GCDAsyncSocket *)sock didConnectToUrl:(NSURL *)url {
    NSLog(@"didConnectToUrl");
}

- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag {
    NSLog(@"didReadData: length = %ld, tag = %ld", data.length, tag);
    [sock writeData:data withTimeout:-1 tag:tag];
    [sock readDataToLength:10 withTimeout:-1 tag:tag + 1];
}

- (void)socket:(GCDAsyncSocket *)sock didReadPartialDataOfLength:(NSUInteger)partialLength tag:(long)tag {
    NSLog(@"didReadPartialDataOfLength: length = %ld, tag = %ld", partialLength, tag);
}

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag {
    NSLog(@"didWriteDataWithTag");
}

- (void)socket:(GCDAsyncSocket *)sock didWritePartialDataOfLength:(NSUInteger)partialLength tag:(long)tag {
    NSLog(@"didWritePartialDataOfLength");
}

#if 0
- (NSTimeInterval)socket:(GCDAsyncSocket *)sock shouldTimeoutReadWithTag:(long)tag
                 elapsed:(NSTimeInterval)elapsed
               bytesDone:(NSUInteger)length {
}

- (NSTimeInterval)socket:(GCDAsyncSocket *)sock shouldTimeoutWriteWithTag:(long)tag
                 elapsed:(NSTimeInterval)elapsed
               bytesDone:(NSUInteger)length {
}
#endif

- (void)socketDidCloseReadStream:(GCDAsyncSocket *)sock {
    NSLog(@"socketDidCloseReadStream");
}

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(nullable NSError *)err {
    NSLog(@"socketDidDisconnect");
}

- (void)socketDidSecure:(GCDAsyncSocket *)sock {
    NSLog(@"socketDidSecure");
    NSData *data = [@"socketDidSecure\n" dataUsingEncoding:NSUTF8StringEncoding];
    [sock writeData:data withTimeout:-1 tag:10];
    [mSocket readDataToLength:10 withTimeout:-1 tag:100];
}

- (void)socket:(GCDAsyncSocket *)sock didReceiveTrust:(SecTrustRef)trust
completionHandler:(void (^)(BOOL shouldTrustPeer))completionHandler {
    NSLog(@"didReceiveTrust");

#if 1
    NSData *certData = [NSData dataWithContentsOfFile:@"/cavan/config/ssl/cert.cer"];

    if (certData) {
        OSStatus status = -1;
        SecTrustResultType result = kSecTrustResultDeny;
        SecCertificateRef cert = SecCertificateCreateWithData(NULL, (__bridge_retained CFDataRef) certData);
        // 设置证书用于验证
        SecTrustSetAnchorCertificates(trust, (__bridge CFArrayRef)[NSArray arrayWithObject:(__bridge id) cert]);
        // 验证服务器证书和本地证书是否匹配
        status = SecTrustEvaluate(trust, &result);
        if ((status == noErr && (result == kSecTrustResultProceed || result == kSecTrustResultUnspecified))) {
            completionHandler(YES);
        } else {
            CFArrayRef arrayRefTrust = SecTrustCopyProperties(trust);
            NSLog(@"error in connection occured\n%@", arrayRefTrust);
            completionHandler(NO);
        }
    } else {
        NSLog(@"local certificates could not be loaded");
        completionHandler(NO);
    }
#else
    completionHandler(YES);
#endif
}
@end
