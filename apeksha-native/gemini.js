import { GoogleGenerativeAI } from "@google/generative-ai";
// import { GoogleGenerativeAI } from "@google/generative-ai";
const genAI = new GoogleGenerativeAI("AIzaSyCnQ-jN4sI_TVHATj0ThzF5ffEIM2Ke3Ms");
const model = genAI.getGenerativeModel({ model: "gemini-1.5-flash" });

const prompt = "Explain how AI works";
async function smth(){
    const result = await model.generateContent(prompt);
    console.log(result.response.text());
}
smth()