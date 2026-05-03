const crypto = require('crypto')

function createCommonToken(params) {
    const access_key = Buffer.from((params.access_key || '').trim(), 'base64')

    const version = (params.version || '').trim()
    const productId = (params.productid || params.product_id || '').trim()
    const deviceName = (params.device_name || params.deviceName || '').trim()
    if (!productId) {
        throw new Error('createCommonToken: productid/product_id is required')
    }

    // 该接口（查询设备属性）一般使用产品级规则签名
    // 若后续接口需要设备级鉴权，可改成 products/{productId}/devices/{deviceName}
    let res = 'products/' + productId
    const et = Math.ceil((Date.now() + 365 * 24 * 3600 * 1000) / 1000)
    const method = 'sha1'

    const key = et + '\n' + method + '\n' + res + '\n' + version

    let sign = crypto.createHmac('sha1', access_key).update(key).digest().toString('base64')

    res = encodeURIComponent(res)
    sign = encodeURIComponent(sign)

    const token = `version=${version}&res=${res}&et=${et}&method=${method}&sign=${sign}`

    return token
}

module.exports = {
  createCommonToken
};


