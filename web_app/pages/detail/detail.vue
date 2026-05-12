<template>
	<!-- 整体布局 -->
	<view class="wrap">
		<!-- 区域一 -->
		<view class="section">
			<view class="section-title">区域一</view>
			<view class="dev-area">
				<!-- 设备卡片 - 温度 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">温度</view>
						<image class="dev-logo" src="../../static/temp.png" mode=""></image>
					</view>
					<view class="dev-data">{{temperature}} ℃</view>
				</view>

				<!-- 设备卡片 - 湿度 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">湿度</view>
						<image class="dev-logo" src="../../static/humi.png" mode=""></image>
					</view>
					<view class="dev-data">{{humidity}} %</view>
				</view>

				<!-- 设备卡片 - CO2 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">CO2</view>
						<image class="dev-logo" src="../../static/co2.png" mode=""></image>
					</view>
					<view class="dev-data">{{CO2}}</view>
				</view>

				<!-- 设备卡片 - 光照 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">光照</view>
						<image class="dev-logo" src="../../static/lamp.png" mode=""></image>
					</view>
					<view class="dev-data">{{light}}</view>
				</view>

				<!-- 设备卡片 - 土壤湿度 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">土壤湿度</view>
						<image class="dev-logo" src="../../static/humi.png" mode=""></image>
					</view>
					<view class="dev-data">{{soil}} %</view>
				</view>
			</view>
		</view>

		<!-- 区域二 -->
		<view class="section">
			<view class="section-title">区域二</view>
			<view class="dev-area">
				<!-- 设备卡片 - 温度1 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">温度</view>
						<image class="dev-logo" src="../../static/temp.png" mode=""></image>
					</view>
					<view class="dev-data">{{temperature1}} ℃</view>
				</view>

				<!-- 设备卡片 - 湿度1 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">湿度</view>
						<image class="dev-logo" src="../../static/humi.png" mode=""></image>
					</view>
					<view class="dev-data">{{humidity1}} %</view>
				</view>

				<!-- 设备卡片 - CO21 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">CO2</view>
						<image class="dev-logo" src="../../static/co2.png" mode=""></image>
					</view>
					<view class="dev-data">{{CO21}}</view>
				</view>

				<!-- 设备卡片 - 光照1 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">光照</view>
						<image class="dev-logo" src="../../static/lamp.png" mode=""></image>
					</view>
					<view class="dev-data">{{light1}}</view>
				</view>

				<!-- 设备卡片 - 土壤湿度1 -->
				<view class="dev-cart">
					<view>
						<view class="dev-name">土壤湿度</view>
						<image class="dev-logo" src="../../static/humi.png" mode=""></image>
					</view>
					<view class="dev-data">{{soil1}} %</view>
				</view>
			</view>
		</view>
		
		<!-- 运动控制区域 -->
		<view class="move-controls">
			<view class="move-buttons">
				<button class="move-button" @click="move(5)">语音报警</button>
				<button class="move-button" :class="{'buzzer-on': buzzer}" @click="toggleBuzzer">蜂鸣器报警</button>
			</view>
		</view>
	</view>
</template>

<script>
	// 引入字符串处理函数
	import {
		stringify
	} from 'querystring';
	// 引入创建通用令牌函数
	const {
		createCommonToken
	} = require('@/key.js')

	// 产品ID和设备名称要替换成自己的
	const product_id = 'dU5jVg1L9b';
	const device_name = 'test';

	// Vue组件导出
	export default {
		// 数据部分
		data() {
			return {
				// 区域一数据
				temperature: '--',
				humidity: '--',
				CO2: '--',
				light: '--',
				soil: '--',
				// 区域二数据
				temperature1: '--',
				humidity1: '--',
				CO21: '--',
				light1: '--',
				soil1: '--',
				// 接口请求token
				token: '',
				// 湿度和温度的阈值
				humi_th: 70,
				temp_th: 28,
				// 控制状态
				led: false,
				buzzer: false,
				Car_flag: null,
				currentAction: '待命',
				key_th: {},
				// 数据更新定时器
				dataTimer: null
			}
		},

		// 页面加载时执行的钩子函数
		onLoad() {
			// 初始化token
			const params = {
				author_key: 'MjZlMmMzNGVmNWI4NDBjYzg4ZDU4OTgyZTdkYjY0ODk=', //用户级秘钥
				version: '2022-05-01',
				user_id: '486732', //用户ID
			}
			this.token = createCommonToken(params);
		},

		// 页面显示时执行的钩子函数
		onShow() {
			this.startDataUpdate();
		},

		// 页面隐藏时执行的钩子函数
		onHide() {
			this.stopDataUpdate();
		},

		// 页面卸载时执行的钩子函数
		onUnload() {
			this.stopDataUpdate();
		},

		// 方法部分
		methods: {
			// 开始数据更新
			startDataUpdate() {
				// 首次获取设备数据
				this.fetchDevData();
				// 定时（每500ms）获取设备数据
				this.dataTimer = setInterval(() => {
					this.fetchDevData();
				}, 500);
			},

			// 停止数据更新
			stopDataUpdate() {
				if (this.dataTimer) {
					clearInterval(this.dataTimer);
					this.dataTimer = null;
				}
			},

			// 获取设备数据的方法
			fetchDevData() {
				// 发送请求获取设备属性
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
					method: 'GET',
					data: {
						product_id: product_id,
						device_name: device_name,
					},
					header: {
						'authorization': this.token
					},
					success: (res) => {
						console.log('设备数据:', res.data);

						// 使用forEach和switch解析所有数据，包括控制参数
						if (res.data.data && Array.isArray(res.data.data)) {
							res.data.data.forEach(item => {
								switch (item.identifier) {
									case 'temperature':
										this.temperature = item.value;
										break;
									case 'humidity':
										this.humidity = item.value;
										break;
									case 'CO2':
										this.CO2 = item.value;
										break;
									case 'light':
										this.light = item.value;
										break;
									case 'soil':
										this.soil = item.value;
										break;
									case 'temperature1':
										this.temperature1 = item.value;
										break;
									case 'humidity1':
										this.humidity1 = item.value;
										break;
									case 'CO21':
										this.CO21 = item.value;
										break;
									case 'light1':
										this.light1 = item.value;
										break;
									case 'soil1':
										this.soil1 = item.value;
										break;
								}
							});
						}


					},
				});
			},

			// 滑动条变化事件的方法
			sliderChange(e, id) {
				console.log(id)
				console.log('value 发生变化：' + e.detail.value)

				// 设置对应滑动条的操作标志
				if (id == 'slider1') {

					this.temp_th = e.detail.value;
					this.key_th = {
						temp_th: this.temp_th,
					};
				} else if (id == 'slider2') {

					this.humi_th = e.detail.value;
					this.key_th = {
						humi_th: this.humi_th,
					};
				}

				// 向后端发送设备属性更新请求
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: this.key_th
					},
					header: {
						'authorization': this.token
					},
				});
			},

			// 消毒开关切换的方法
			onLedSwitch(event) {
				// 正确获取开关值并打印
				let value = event.detail.value;


				this.led = value; // 立即更新本地状态

				// 发送请求更新设备属性
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: {
							"led": value
						}
					},
					header: {
						'authorization': this.token
					}

				});
			},
			// 运动控制
			move(action) {
				const actions = ['前进', '左转', '停止', '右转', '后退', '避障模式', '循迹模式'];
				this.currentAction = actions[action];
				this.Car_flag = action;

				this.uploadCarFlag();
			},

			// 上传Car_flag
			uploadCarFlag() {
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: {
							"Car_flag": this.Car_flag
						}
					},
					header: {
						'authorization': this.token
					},
				});
			},

			toggleBuzzer() {
				this.buzzer = !this.buzzer;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: {
							"SET1": this.buzzer ? 1 : 0
						}
					},
					header: {
						'authorization': this.token
					},
				});
			}
		}
	}
</script>


<style>
	/* 整体页面容器样式 */
	.wrap {
		padding: 30rpx;
		background-color: #4a90d9;
		min-height: 100vh;
		display: flex;
		flex-wrap: wrap;
		justify-content: space-between;
	}

	/* 区域样式 */
	.section {
		width: calc(50% - 15rpx);
		margin-bottom: 40rpx;
	}

	/* 区域标题样式 */
	.section-title {
		font-size: 32rpx;
		font-weight: bold;
		color: #fff;
		margin-bottom: 20rpx;
		padding-left: 10rpx;
		border-left: 8rpx solid #fff;
	}

	/* 设备区域样式 */
	.dev-area {
		display: flex;
		/* 使用弹性盒子布局 */
		justify-content: space-between;
		/* 在弹性容器中均匀分布子元素，两端对齐 */
		flex-wrap: wrap;
		/* 如果子元素溢出容器，则折叠到下一行 */
	}

	/* 设备卡片样式 */
	.dev-cart {
		height: 100rpx;
		width: calc(50% - 10rpx);
		border-radius: 20rpx;
		margin-top: 15rpx;
		display: flex;
		justify-content: space-around;
		align-items: center;
		box-shadow: 0 0 10rpx #ccc;
	}

	/* 长设备卡片样式 */
	.device-cart-l {
		height: 150rpx;
		/* 设置高度为150像素 */
		width: 700rpx;
		/* 设置宽度为700像素 */
		border-radius: 30rpx;
		/* 设置边框圆角为30像素 */
		margin-top: 30rpx;
		/* 设置上外边距为30像素 */
		display: flex;
		/* 使用弹性盒子布局 */
		justify-content: space-around;
		/* 在弹性容器中均匀分布子元素，两端对齐 */
		align-items: center;
		/* 在弹性容器中垂直居中对齐子元素 */
		box-shadow: 0 0 15rpx #ccc;
		/* 设置盒子阴影，颜色为灰色 */
	}

	/* 设备名称样式 */
	.dev-name {
		font-size: 20rpx;
		/* 设置字体大小为20像素 */
		text-align: center;
		/* 文本居中对齐 */
		color: #6d6d6d;
		/* 字体颜色为灰色 */
	}

	/* 设备图标样式 */
	.dev-logo {
		width: 40rpx;
		height: 40rpx;
		margin-top: 8rpx;
	}

	/* 设备数据样式 */
	.dev-data {
		font-size: 28rpx;
		color: #6d6d6d;
	}

	/* 滑动条样式 */
	.ctrl-slider {
		width: 580rpx;
		/* 设置宽度为580像素 */
	}

	/* 运动控制区域样式 */
	.move-controls {
		margin-top: 50rpx;
		display: flex;
		flex-direction: column;
		align-items: center;
	}

	.move-buttons {
		display: flex;
		justify-content: center;
		width: 100%;
	}

	.move-button,
	.turn-button {
		width: 210rpx;
		height: 110rpx;
		margin: 10rpx;
		font-size: 20rpx;
		background-color: #4CAF50;
		color: #fff;
	}

	.buzzer-on {
		background-color: #f44336 !important;
	}

	.turning-buttons {
		display: flex;
		justify-content: space-between;
		width: 100%;
		margin-top: 10rpx;
	}

	.current-action {
		margin-top: 20rpx;
		font-size: 18rpx;
		color: #333;
		width: 100%;
		text-align: center;
	}
</style>