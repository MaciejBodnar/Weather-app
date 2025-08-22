const input = document.querySelector("input");
const form = document.querySelector("form");
const temp = document.querySelector(".temp");
const apiKey = `639380fcdcef95c3d2c0b54aa968f122`;
const humidity = document.querySelector(".humidity");
const cityName = document.querySelector(".city");
const wind = document.querySelector(".wind");
const weatherIcon = document.querySelector(".weather-icon");

form.addEventListener("submit", async (event) => {
  event.preventDefault();
  const city = input.value;

  try {
    const response = await fetch(
      `https://api.openweathermap.org/data/2.5/weather?q=${city.toLowerCase()}&appid=${apiKey}&units=metric`
    );
    const data = await response.json();
    temp.textContent = Math.round(data.main.temp) + "°C";
    humidity.textContent = data.main.humidity + "%";
    wind.textContent = Math.round(data.wind.speed) + "km/h";
    cityName.textContent = `${data.name}`;
    input.value = "";

    if (data.weather[0].main === "Clear") {
      weatherIcon.src = "images/clear.png";
    } else if (data.weather[0].main === "Clouds") {
      weatherIcon.src = "images/clouds.png";
    } else if (data.weather[0].main === "Drizzle") {
      weatherIcon.src = "images/drizzle.png";
    } else if (data.weather[0].main === "Humidity") {
      weatherIcon.src = "images/humidity.png";
    } else if (data.weather[0].main === "Rain") {
      weatherIcon.src = "images/rain.png";
    } else if (data.weather[0].main === "Mist") {
      weatherIcon.src = "images/mist.png";
    } else if (data.weather[0].main === "Snow") {
      weatherIcon.src = "images/snow.png";
    } else {
      weatherIcon.src = "images/wind.png";
    }
  } catch (error) {
    alert("Enter a valid city name");
  }
});
