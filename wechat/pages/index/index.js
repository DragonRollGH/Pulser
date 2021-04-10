var MD5 = require("../../utils/MD5.js");

Page({
  users: [
    "cf18f1fce422fbbccbda72227441bf41",
    "725e1844a68f9e6ac3b2bff4728709ee"
  ],
  verify: function(inputname) {
    let errorUsername = true
    for(let i in this.users) {
      if (MD5.md5(MD5.md5(inputname)) == this.users[i]) {
        errorUsername = false
        this.setData({
          errorUsername: false
        })
        // wx.navigateTo({
        wx.redirectTo({
          url: '../pulser/pulser?user=' + inputname
        })
      }
    }
    if (errorUsername) {
      this.setData({
        errorUsername: true
      })
    }
  },
  login(event) {
    let inputname = event.detail.value.name
    wx.setStorageSync("inputname", inputname)
    this.verify(inputname)
  },
  onLoad() {
    let inputname = wx.getStorageSync("inputname") || ""
    if (inputname) {
      this.verify(inputname)
    }
  }
})
