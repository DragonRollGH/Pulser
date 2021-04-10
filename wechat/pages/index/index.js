// index.js
// 获取应用实例
var MD5 = require('../../utils/MD5.js');

const app = getApp()

Page({
  // data: {
  //   motto: 'Hello World',
  //   userInfo: {},
  //   hasUserInfo: false,
  //   canIUse: wx.canIUse('button.open-type.getUserInfo'),
  //   canIUseGetUserProfile: false,
  //   canIUseOpenData: wx.canIUse('open-data.type.userAvatarUrl') && wx.canIUse('open-data.type.userNickName') // 如需尝试获取用户信息可改为false
  // },
  // // 事件处理函数
  // bindViewTap() {
  //   wx.navigateTo({
  //     url: '../logs/logs'
  //   })
  // },
  onLoad() {
    if (wx.getUserProfile) {
      this.setData({
        canIUseGetUserProfile: true
      })
    }
  },
  // getUserProfile(e) {
  //   // 推荐使用wx.getUserProfile获取用户信息，开发者每次通过该接口获取用户个人信息均需用户确认，开发者妥善保管用户快速填写的头像昵称，避免重复弹窗
  //   wx.getUserProfile({
  //     desc: '展示用户信息', // 声明获取用户个人信息后的用途，后续会展示在弹窗中，请谨慎填写
  //     success: (res) => {
  //       console.log(res)
  //       this.setData({
  //         userInfo: res.userInfo,
  //         hasUserInfo: true
  //       })
  //     }
  //   })
  // },
  login(e){
    var username = MD5.md5(MD5.md5(e.detail.value.name))
    if (username == "cf18f1fce422fbbccbda72227441bf41") {
      wx.setStorage({
        data: cf18f1fce422fbbccbda72227441bf41,
        key: 'username',
      })
      console.log("Y1")
    }
    else if (username == "725e1844a68f9e6ac3b2bff4728709ee") {
      wx.setStorage({
        data: cf18f1fce422fbbccbda72227441bf41,
        key: 'username',
      })
      console.log("Y2")
    }
    else {
      console.log("N")
    }
  }
})
