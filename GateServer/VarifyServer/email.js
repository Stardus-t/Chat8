const nodemailer = require('nodemailer');
const config_module = require("./config")

/**
 * 创建发送邮件的代理
 */
let transport = nodemailer.createTransport({
    host: 'smtp.163.com',
    port: 465,
    secure: true,
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass // 邮箱授权码或者密码
    }
});

/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_){
    const mailOptions = {
        ...mailOptions_,
        from: config_module.email_user // 显式指定发件人，与授权邮箱一致
    };
    return new Promise(function(resolve, reject){
        // 关键修正：传入处理后的 mailOptions，而不是原始的 mailOptions_
        transport.sendMail(mailOptions, function(error, info){
            if (error) {
                console.log(error);
                reject(error);
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)
            }
        });
    })
   
}

module.exports.SendMail = SendMail