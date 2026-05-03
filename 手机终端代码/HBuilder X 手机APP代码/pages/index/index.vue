<template>
	<view class="wrap" :class="themeClass">
		<!-- 顶部栏 -->
		<view class="header">
			<view class="theme-item">
				<text class="theme-label">深色模式</text>
				<switch :checked="isDark" @change="onThemeSwitch" color="#5271ff" />
			</view>
			<view class="time-box">
				<text class="date">{{ currentDate }}</text>
				<text class="time">{{ currentTime }}</text>
			</view>
		</view>

		<view class="device-grid">
			<!-- 温度 -->
			<view class="card">
				<view class="card-left">
					<image class="icon" src="/static/temp.png" mode=""></image>
					<view class="name">温度</view>
				</view>
				<view class="value">{{temp}}℃</view>
			</view>

			<!-- 湿度 -->
			<view class="card">
				<view class="card-left">
					<image class="icon" src="/static/humi.png" mode=""></image>
					<view class="name">湿度</view>
				</view>
				<view class="value">{{huim}}%</view>
			</view>

			<!-- 台灯 -->
			<view class="card">
				<view class="card-left">
					<image class="icon" src="/static/light.png" mode=""></image>
					<view class="name">台灯</view>
				</view>
				<switch :checked="led" @change="onLedSwitch" color="#5271ff"/>
			</view>

			<!-- 大门 -->
			<view class="card">
				<view class="card-left">
					<image class="icon" src="/static/door.png" mode=""></image>
					<view class="name">大门</view>
				</view>
				<switch :checked="door" @change="onDoorSwitch" color="#5271ff"/>
			</view>

			<!-- 风扇 -->
			<view class="card">
				<view class="card-left">
					<image class="icon" src="/static/fan.png" mode=""></image>
					<view class="name">风扇</view>
				</view>
				<switch :checked="fan" @change="onFanSwitch" color="#5271ff"/>
			</view>

			<!-- 窗帘 -->
			<view class="card">
				<view class="card-left">
					<image class="icon" src="/static/curtain.png" mode=""></image>
					<view class="name">窗帘</view>
				</view>
				<switch :checked="curtians" @change="onCurtainsSwitch" color="#5271ff"/>
			</view>

			<!-- 状态条 -->
			<view class="card card-full">
				<view class="status-row">
					<text class="status-label">设备状态</text>
					<view :class="['status-dot', getConnectionLightClass(connectionStatus)]"></view>
					<text class="status-text">{{connectionStatus}}</text>
				</view>
			</view>
		</view>
	</view>
</template>

<script>
	const { createCommonToken } = require('@/key.js')
	export default {
		data() {
			return {
				temp: '26',
				huim: '45',
				led: true,
				fan: false,
				curtians: false,
				door: false,
				isUpdatingLed: false,
				isUpdatingDoor: false,
				isUpdatingFan: false,
				isUpdatingCurtains: false,
				
				isDark: false,
				token: '',
				connectionStatus: '在线',
				overheatState: '1',
				smokeSensorState: '1',
				productId: '',				//根据onenet平台上创建的设备来填写productId
				deviceName: '',				//根据onenet平台上创建的设备来填写deviceName
				accessKey: '',				//根据onenet平台上创建的设备来填写accessKey

				currentDate: '',
				currentTime: '',
				timeTimer: null
			}
		},
		computed: {
			themeClass() {
				return this.isDark ? 'dark-theme' : 'light-theme';
			}
		},
		onLoad() {
			const savedTheme = uni.getStorageSync('app_theme');
			this.isDark = savedTheme === 'dark';

			this.updateTime();
			this.timeTimer = setInterval(() => {
				this.updateTime();
			}, 1000);

			const params = {
				access_key: this.accessKey,
				version: '2022-05-01',
				product_id: this.productId,
				device_name: this.deviceName,
				user_id: this.productId,
			}
			this.token = createCommonToken(params);
		},
		onShow() {
			this.fetchDevData();
			this.timer = setInterval(() => {
				this.fetchDevData();
			}, 3000)
		},
		onHide() {
			clearInterval(this.timer)
		},
		onUnload() {
			clearInterval(this.timeTimer)
		},
		methods: {
			updateTime() {
				let d = new Date();
				let pad = n=>n<10?'0'+n:n;
				this.currentDate = `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())}`;
				this.currentTime = `${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
			},
			onThemeSwitch(e) {
				this.isDark = e.detail.value;
				uni.setStorageSync('app_theme', this.isDark ? 'dark' : 'light');
				uni.showToast({
					title: this.isDark ? "已开启深色模式" : "已关闭深色模式",
					icon: "none", duration:700
				});
			},
			fetchDevData() {
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
					method: 'GET',
					data: { product_id: this.productId, device_name: this.deviceName },
					header: { 'authorization': this.token },
					success: (res) => {
					  const props = res?.data?.data ?? [];
					  const getProp = (key) => {
					    const item = props.find(p => 
					      p.id === key || p.identifier === key || p.name === key || p.key === key
					    );
					    return item?.value;
					  };
					  const toBool = (val) => String(val).toLowerCase() === 'true';
					  this.huim = getProp('humi')??getProp('humidity')??this.huim;
					  this.temp = getProp('temp')??getProp('temperature')??this.temp;
					  if(!this.isUpdatingLed) this.led = toBool(getProp('led'))??this.led;
					  if(!this.isUpdatingDoor) this.door = toBool(getProp('door'))??this.door;
					  if(!this.isUpdatingFan) this.fan = toBool(getProp('fan'))??this.fan;
					  if(!this.isUpdatingCurtains) this.curtians = toBool(getProp('curtains'))??this.curtians;
					  const statusProp = getProp('status')??getProp('online')??getProp('connect_status');
					  if(statusProp!==undefined) this.updateConnectionStatus(statusProp);
					}
				});
				uni.request({
					url: 'https://iot-api.heclouds.com/device/detail',
					method: 'GET',
					data: { product_id: this.productId, device_name: this.deviceName },
					header: { 'authorization': this.token },
					success: (res) => {
						const status = res?.data?.data?.status;
						this.updateConnectionStatus(status);
					},
					fail:()=>{ this.connectionStatus = '未知'; }
				});
			},
			updateConnectionStatus(status) {
				if(status===1||status==='1'||status===true) this.connectionStatus='在线';
				else if(status===2||status==='2') this.connectionStatus='未激活';
				else if(status===0||status==='0'||status===false) this.connectionStatus='离线';
				else this.connectionStatus='未知';
			},
			getStateText(state) { return String(state)==='1'?'正常':'报警'; },
			getStateLightClass(state) { return String(state)==='1'?'status-green':'status-red'; },
			getConnectionLightClass(status) { return status==='在线'?'status-green':'status-red'; },
			onLedSwitch(e) {
				const v=e.detail.value;
				this.isUpdatingLed=true; this.led=v;
				uni.showToast({title:v?"台灯已开启":"台灯已关闭",icon:"none",duration:700});
				uni.request({
					url:'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method:'POST',
					data:{product_id:this.productId,device_name:this.deviceName,params:{led:v}},
					header:{'authorization':this.token},
					success:()=>{setTimeout(()=>{this.isUpdatingLed=false;},2000);},
					fail:()=>{this.led=!v;this.isUpdatingLed=false;uni.showToast({title:'控制失败',icon:'none'});}
				});
			},
			onDoorSwitch(e) {
				const v=e.detail.value;
				this.isUpdatingDoor=true; this.door=v;
				uni.showToast({title:v?"大门已开启":"大门已关闭",icon:"none",duration:700});
				uni.request({
					url:'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method:'POST',
					data:{product_id:this.productId,device_name:this.deviceName,params:{door:v}},
					header:{'authorization':this.token},
					success:()=>{setTimeout(()=>{this.isUpdatingDoor=false;},2000);},
					fail:()=>{this.door=!v;this.isUpdatingDoor=false;uni.showToast({title:'控制失败',icon:'none'});}
				});
			},
			onFanSwitch(e) {
				const v=e.detail.value;
				this.isUpdatingFan=true; this.fan=v;
				uni.showToast({title:v?"风扇已开启":"风扇已关闭",icon:"none",duration:700});
				uni.request({
					url:'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method:'POST',
					data:{product_id:this.productId,device_name:this.deviceName,params:{fan:v}},
					header:{'authorization':this.token},
					success:()=>{setTimeout(()=>{this.isUpdatingFan=false;},2000);},
					fail:()=>{this.fan=!v;this.isUpdatingFan=false;uni.showToast({title:'控制失败',icon:'none'});}
				});
			},
			onCurtainsSwitch(e) {
				const v=e.detail.value;
				this.isUpdatingCurtains=true; this.curtians=v;
				uni.showToast({title:v?"窗帘已开启":"窗帘已关闭",icon:"none",duration:700});
				uni.request({
					url:'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method:'POST',
					data:{product_id:this.productId,device_name:this.deviceName,params:{curtains:v}},
					header:{'authorization':this.token},
					success:()=>{setTimeout(()=>{this.isUpdatingCurtains=false;},2000);},
					fail:()=>{this.curtians=!v;this.isUpdatingCurtains=false;uni.showToast({title:'控制失败',icon:'none'});}
				});
			}
		}
	}
</script>

<style>
	.light-theme {
		--bg: #f5f7fa;
		--card: #ffffff;
		--text1: #2d2f33;
		--text2: #838690;
		--shadow1: rgba(0,0,0,0.05);
		--shadow2: rgba(255,255,255,0.7);
		--green: #4cd964;
		--red: #ff3b30;
	}
	.dark-theme {
		--bg: #1c1c28;
		--card: #2b2d3a;
		--text1: #f0f0f0;
		--text2: #a1a5b9;
		--shadow1: rgba(0,0,0,0.2);
		--shadow2: rgba(255,255,255,0.03);
		--green: #4cd964;
		--red: #ff3b30;
	}

	.wrap {
		padding: 30rpx;
		background: var(--bg);
		min-height: 100vh;
		box-sizing: border-box;
	}

	/* 顶部 */
	.header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 30rpx;
	}
	.theme-item {
		display: flex;
		align-items: center;
		gap: 20rpx;
		padding: 20rpx 28rpx;
		background: var(--card);
		border-radius: 40rpx;
		box-shadow: 4rpx 4rpx 12rpx var(--shadow1), -4rpx -4rpx 12rpx var(--shadow2);
	}
	.theme-label {
		font-size: 26rpx;
		color: var(--text2);
	}
	.time-box {
		display: flex;
		flex-direction: column;
		align-items: flex-end;
		padding: 16rpx 24rpx;
		background: var(--card);
		border-radius: 24rpx;
		box-shadow: 4rpx 4rpx 12rpx var(--shadow1), -4rpx -4rpx 12rpx var(--shadow2);
	}
	.date {
		font-size: 24rpx;
		color: var(--text2);
	}
	.time {
		font-size: 30rpx;
		font-weight: 600;
		color: var(--text1);
		margin-top: 4rpx;
	}

	/* 设备网格 */
	.device-grid {
		display: flex;
		flex-wrap: wrap;
		justify-content: space-between;
		gap: 24rpx;
	}
	.card {
		width: 320rpx;
		height: 180rpx;
		background: var(--card);
		border-radius: 32rpx;
		padding: 0 32rpx;
		box-sizing: border-box;
		display: flex;
		align-items: center;
		justify-content: space-between;
		box-shadow: 4rpx 4rpx 12rpx var(--shadow1), -4rpx -4rpx 12rpx var(--shadow2);
	}
	.card-full {
		width: 100%;
		height: 120rpx;
		justify-content: flex-start;
	}
	.card-left {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 8rpx;
	}
	.icon {
		width: 70rpx;
		height: 70rpx;
		object-fit: contain;
	}
	.name {
		font-size: 26rpx;
		color: var(--text2);
	}
	.value {
		font-size: 52rpx;
		font-weight: 700;
		color: var(--text1);
	}

	/* 状态行 */
	.status-row {
		display: flex;
		align-items: center;
		gap: 20rpx;
	}
	.status-label {
		font-size: 28rpx;
		color: var(--text2);
	}
	.status-dot {
		width: 24rpx;
		height: 24rpx;
		border-radius: 50%;
	}
	.status-green {
		background: var(--green);
	}
	.status-red {
		background: var(--red);
	}
	.status-text {
		font-size: 28rpx;
		color: var(--text1);
		font-weight: 500;
	}

	switch {
		transform: scale(1.15);
	}
</style>